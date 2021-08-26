import React, {
    createContext,
    CSSProperties,
    Dispatch,
    ProfilerOnRenderCallback, SetStateAction,
    useContext,
    useMemo,
    useReducer,
    useState
} from "react";
import {StackObject} from "./components/MathQuillField";
import {Fxn} from "./GlamCore";

export enum ProtofunctionType {
    C,
    R2C,
    C2C,
}

export class Protofunction {
    name: string
    id: number
    parameterName: string = ""
    style: CSSProperties = {stroke: "white"}
    type: ProtofunctionType = ProtofunctionType.C
    text: string = ""
    stack: StackObject[] = []
    functionName: string = ""
    jitFunction?: Fxn
    drawing: boolean = false

    constructor(_id: number, _name: string) {
        this.id = _id
        this.name = _name
    }
}


type ProtofunctionContextType = Record<number, Protofunction>

const ProtofunctionContext = React.createContext<[ProtofunctionContextType, Dispatch<UpdateProtofunctionAction>] | undefined>(undefined)

interface UpdateProtofunctionAction {
    pfId: number
    change: any
}

export const ProtofunctionContextProvider: React.FC = (props) => {
    const [ctx, dispatch] = useReducer((oldCtx: ProtofunctionContextType, update: UpdateProtofunctionAction) => {
        let newCtx = {...oldCtx}

        newCtx[update.pfId] = {
            ...(oldCtx[update.pfId] ? oldCtx[update.pfId] : new Protofunction(update.pfId, "New Function")),
            ...update.change
        }
        return newCtx
    }, {})

    return <ProtofunctionContext.Provider value={[ctx, dispatch]}>
        {props.children}
    </ProtofunctionContext.Provider>
}

export function useProtofunction(pfId: number, type?: ProtofunctionType): [Protofunction, Dispatch<any>] {
    const ctxValue = useContext(ProtofunctionContext)
    if (!ctxValue) {
        throw "Protofunction context not provided!"
    }

    const [ctx, dispatch] = ctxValue
    if (!ctx[pfId]) {
        dispatch({pfId: pfId, change: {type: type}})
        ctx[pfId] = new Protofunction(pfId, "New Function")
    }

    return [ctx[pfId], (change: any) => dispatch({pfId: pfId, change: change})]
}

export function useProtofunctionCreator(): [number, Dispatch<ProtofunctionType>] {
    const ctxValue = useContext(ProtofunctionContext)
    if (!ctxValue) {
        throw "Protofunction context not provided!"
    }

    const [ctx, dispatch] = ctxValue

    return [Object.values(ctx).length, (type) => dispatch({pfId: Object.values(ctx).length, change: {type: type}})]
}
