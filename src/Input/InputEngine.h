#pragma once

#include "Input/Layer/Layer.h"
#include "Input/Layer/LayerGroup.h"
#include "Input/Layer/MainLayerGroup.h"
#include "Input/InputMode.h"
#include "Display/Workspace.h"
#include "Input/Layer/LayerContext.h"

class InputEngine
{
    public:
        InputContext ctx;

        UIState& ui;

        LayerContext layerContext;

        Sequencer& sequencer;
        RotaryEncoder& rotaryEncoders;

        MainLayerGroup mainLayerGroup;

        LayerGroup* currentLayerGroup = nullptr;

        Layer* currentLayer = nullptr;

        // Layer ayant reçu le ButtonPressed de chaque touche Fn.
        // Permet de garantir que les événements Hold et Tap
        // sont toujours envoyés au même layer, même si le layer
        // actif change pendant que le bouton est maintenu.
        Layer* pressedLayer[Constants::NUMBER_OF_BUTTONS] = {nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr};

        explicit InputEngine(Sequencer& _sequencer, RotaryEncoder& _rotaryEncoders, UIState& _ui)
            :
            ui(_ui),
            layerContext(_sequencer, _rotaryEncoders),
            sequencer(_sequencer),
            rotaryEncoders(_rotaryEncoders),
            mainLayerGroup(ctx, layerContext, ui)
        {
        }

        void begin()
        {
            resolveCurrentLayer();
            currentLayer->applyEncoderMapping();
            currentLayer->applyEncoderValues();
            ui.requestRedraw();
        }

        void resolveLayerGroup()
        {
            switch (ui.workspace)
            {
                case Workspace::Sequencer:
                    currentLayerGroup = &mainLayerGroup;

                    break;
            }
        }

        // Détermine le layer actif à partir du contexte courant.
        // Si le layer change, appelle automatiquement
        // onExit() sur l'ancien puis onEnter() sur le nouveau.
        void resolveCurrentLayer()
        {
            resolveLayerGroup();

            Layer* newLayer = currentLayerGroup->layer();

            if (newLayer == currentLayer)
                return;

            if (currentLayer)
                currentLayer->onExit(newLayer);

            Layer* previous = currentLayer;
            currentLayer = newLayer;

            if (currentLayer)
                currentLayer->onEnter(previous);
        }

    void handleEvent(InputEvent& event)
    {
        // Associe l'événement physique (bouton/encodeur)
        // à un ControlId logique selon le LayerGroup courant.  
        switch (event.type)
        {
            case InputEventType::EncoderTurned:
            {
                event.control = currentLayerGroup->encoderToControl(event.id);
                break;
            }
            case InputEventType::ButtonPressed:
            case InputEventType::ButtonHold:
            case InputEventType::ButtonReleased:
            {
                event.control = currentLayerGroup->buttonToControl(event.id);
                break;
            }
        }

        // Synchronise le layer actif avec le contexte actuel.
        // Cela permet par exemple de quitter un ModalLayer
        // qui aurait été fermé par l'événement précédent.
        resolveCurrentLayer();

        // Met à jour l'état des touches (Fn, Step...)
        // avant le traitement de l'événement.
        updateContext(event);

        int8_t fn = fnIndex(event.control);

        switch (event.type)
        {
            case InputEventType::ButtonPressed:
            {
                // Mémorise le layer ayant reçu l'appui.
                // Les événements Hold/Tap devront être renvoyés
                // au même layer même si le layer actif change entre-temps.
                if (fn >= 0) {
                    pressedLayer[fn] = currentLayer;
                }

                if (isStep(event.control))
                {
                    // Toute Fn déjà maintenue devient un modificateur
                    // lorsqu'un Step est pressé.
                    for (uint8_t i = 0; i < 8; i++)
                    {
                        if (ctx.fnMask & (1 << i))
                            ctx.fnState[i].usedAsModifier = true;
                    }

                    currentLayer->onStepPressed(event);
                }

                break;
            }

            case InputEventType::ButtonHold:
            {
                // Le Hold est envoyé au layer qui a reçu le ButtonPressed.
                // Cela évite qu'un changement de layer pendant le maintien
                // modifie le comportement.
                if (fn >= 0)
                {
                    if (!ctx.fnState[fn].consumed) {
                        if (pressedLayer[fn]) {
                            pressedLayer[fn]->onButtonHold(event);
                        }
                        ctx.fnState[fn].holdTriggered = true;
                    }
                }

                break;
            }

            case InputEventType::ButtonReleased:
            {
                // Les événements StepReleased sont envoyés
                // uniquement si le Step n'a pas servi de modificateur.
                if (isStep(event.control))
                {
                    if (!ctx.stepUsedAsModifier){
                        currentLayer->onStepReleased(event);
                    }

                    ctx.stepUsedAsModifier = false;
                }

                // Un Tap est envoyé uniquement si le bouton
                // n'a été ni utilisé comme modificateur
                // ni transformé en Hold.
                if (fn >= 0)
                {
                    if (!ctx.fnState[fn].usedAsModifier &&
                        !ctx.fnState[fn].holdTriggered)
                    {
                        if (pressedLayer[fn]) {
                            pressedLayer[fn]->onButtonTap(event);
                        }
                    }

                    // Nettoyage de l'état Fn.
                    ctx.fnState[fn].usedAsModifier = false;
                    ctx.fnState[fn].holdTriggered = false;
                    ctx.fnState[fn].consumed = false;

                    pressedLayer[fn] = nullptr;
                }

                break;
            }

            case InputEventType::EncoderTurned:
            {
                // Dès qu'un encodeur est tourné,
                // toutes les Fn actuellement maintenues
                // deviennent des modificateurs.
                for (uint8_t i = 0; i < 8; i++)
                {
                    if (ctx.fnMask & (1 << i))
                        ctx.fnState[i].usedAsModifier = true;
                }

                // Un Step maintenu devient également
                // un modificateur.
                if (ctx.stepHeld) {
                    ctx.stepUsedAsModifier = true;
                }

                // Les Fn ayant servi de modificateur
                // ne pourront plus générer de Tap.
                for (uint8_t i = 0; i < 8; i++)
                {
                    if (ctx.fnMask & (1 << i)) {
                        ctx.fnState[i].consumed = true;
                    }
                }

                currentLayer->onEncoder(event);
                break;
            }
        }
        // Le traitement précédent a pu modifier le contexte
        // (Fn pressée, Step relâché, fermeture d'une modal...).
        // On résout donc le layer actif pour les prochains événements.
        resolveCurrentLayer();

        // Le layer actif applique son mapping d'encodeurs
        // et synchronise leurs valeurs.
        currentLayer->applyEncoderMapping();
        currentLayer->applyEncoderValues();

        // Demande le rafraîchissement de l'interface.
        ui.requestRedraw();
    }

    void updateContext(const InputEvent& event)
    {
        switch (event.type)
        {
            case InputEventType::ButtonPressed:

                if (isFn(event.control))
                    ctx.fnMask |= fnBit(event.control);

                if (isStep(event.control))
                {
                    ctx.stepHeld = true;
                    ctx.stepId = event.control;
                }

                break;

            case InputEventType::ButtonReleased:

                if (isFn(event.control))
                    ctx.fnMask &= ~fnBit(event.control);

                if (isStep(event.control))
                    ctx.stepHeld = false;

                break;

            default:
                break;
        }
    }
};


/*
void handleEvent(InputEvent& event)
{
    // Associe l'événement physique (bouton/encodeur)
    // à un ControlId logique selon le LayerGroup courant.
    switch (event.type)
    {
        ...
    }

    // Synchronise le layer actif avec le contexte actuel.
    // Cela permet par exemple de quitter un ModalLayer
    // qui aurait été fermé par l'événement précédent.
    resolveCurrentLayer();

    // Met à jour l'état des touches (Fn, Step...)
    // avant le traitement de l'événement.
    updateContext(event);

    int8_t fn = fnIndex(event.control);

    switch (event.type)
    {
        case InputEventType::ButtonPressed:
        {
            // Mémorise le layer ayant reçu l'appui.
            // Les événements Hold/Tap devront être renvoyés
            // au même layer même si le layer actif change entre-temps.
            if (fn >= 0)
                pressedLayer[fn] = currentLayer;

            if (isStep(event.control))
            {
                // Toute Fn déjà maintenue devient un modificateur
                // lorsqu'un Step est pressé.
                ...

                currentLayer->onStepPressed(event);
            }

            break;
        }

        case InputEventType::ButtonHold:
        {
            // Le Hold est envoyé au layer qui a reçu le ButtonPressed.
            // Cela évite qu'un changement de layer pendant le maintien
            // modifie le comportement.
            ...

            break;
        }

        case InputEventType::ButtonReleased:
        {
            // Les événements StepReleased sont envoyés
            // uniquement si le Step n'a pas servi de modificateur.
            ...

            // Un Tap est envoyé uniquement si le bouton
            // n'a été ni utilisé comme modificateur
            // ni transformé en Hold.
            ...

            // Nettoyage de l'état Fn.
            ...

            break;
        }

        case InputEventType::EncoderTurned:
        {
            // Dès qu'un encodeur est tourné,
            // toutes les Fn actuellement maintenues
            // deviennent des modificateurs.
            ...

            // Un Step maintenu devient également
            // un modificateur.
            ...

            // Les Fn ayant servi de modificateur
            // ne pourront plus générer de Tap.
            ...

            currentLayer->onEncoder(event);

            break;
        }
    }

    // Le traitement précédent a pu modifier le contexte
    // (Fn pressée, Step relâché, fermeture d'une modal...).
    // On résout donc le layer actif pour les prochains événements.
    resolveCurrentLayer();

    // Le layer actif applique son mapping d'encodeurs
    // et synchronise leurs valeurs.
    currentLayer->applyEncoderMapping();
    currentLayer->applyEncoderValues();

    // Demande le rafraîchissement de l'interface.
    ui.requestRedraw();
}

void resolveCurrentLayer()
{
    // Détermine le layer actif à partir du contexte courant.
    // Si le layer change, appelle automatiquement
    // onExit() sur l'ancien puis onEnter() sur le nouveau.
    ...
}

// Layer ayant reçu le ButtonPressed de chaque touche Fn.
// Permet de garantir que les événements Hold et Tap
// sont toujours envoyés au même layer, même si le layer
// actif change pendant que le bouton est maintenu.
Layer* pressedLayer[Constants::NUMBER_OF_BUTTONS] = { ... };

*/
