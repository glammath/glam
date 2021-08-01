import React, {Dispatch, Reducer, useContext, useMemo, useReducer} from "react";
import {SigArcFocus, Signal, SS} from "./Signals";
import {complex, ptr} from "./GlamCore";

export type functionSupplier = (minX: number, minY: number, maxX: number, maxY: number) => complex[]

export class GlamAppState {
    limits: [number, number, number, number] = [-4, -4, 4, 4]

    focusedArc: ptr = 0
    selectedArc: ptr = 0
    drawList: number[] = []
}

interface ContextContainer {
    _glamState: GlamAppState
}

const signalReducer: Reducer<GlamAppState, Signal> = (state, action) => {
    let newState = state

    type signalDataType = Parameters<typeof action._dataType>[0]
    const signal = action as signalDataType
    console.log("DEBUG: signal " + JSON.stringify(action));

    switch (action.signalName) {
        case "Ready":
            break;
        case "ArcFocus":
            console.debug(`focus arc: ${signal.arc}`)
            newState.focusedArc = signal.arc
            break;
    }

    (window as unknown as ContextContainer)._glamState = newState
    return newState;
}

export const SignalContext = React.createContext(null as unknown as [GlamAppState, Dispatch<any>])

export const GlamAppContextProvider: React.FC = (props) => {
    const [state, dispatch] = useReducer(signalReducer, new GlamAppState())

    return (
        <SignalContext.Provider value={[state, dispatch]}>
        {props.children}
    </SignalContext.Provider>)
}
