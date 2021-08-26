import React, {useEffect, useRef, useState} from "react";
import {ProtofunctionType, useProtofunction} from "../ProtofunctionContext";
import {Button, Intent} from "@blueprintjs/core";
import "@blueprintjs/core/lib/css/blueprint.css"
import "@blueprintjs/icons/lib/css/blueprint-icons.css"
import "../BP3Restyler.css"
import {InfoDialog} from "./InfoDialog";
import {NewArcIcon, NewPointIcon} from "./Icons";
import {Fxn} from "../GlamCore";
import {generateFunctionName, MathQuillField} from "./MathQuillField";

interface FunctionEntryProps {
    n: number
    removeCallback: () => void
    drawCallback: (id: number, remove: boolean) => void
    type: ProtofunctionType
}

const jitCache: Record<string, Fxn> = {}

export const FunctionEntry: React.FC<FunctionEntryProps> = (props) => {
    const [pf, updatePf] = useProtofunction(props.n, props.type)
    const inputRef = useRef<HTMLInputElement>(null)
    const prettyRef = useRef<HTMLDivElement>(null)
    const [compiling, setCompiling] = useState(false)
    const [functionSourceOpen, setFunctionSourceOpen] = useState(false)
    const [error, setError] = useState<string | null>(null)

    useEffect(() => {
        if (pf.drawing) {
            if (!pf.jitFunction || (pf.jitFunction.getName() !== pf.functionName)) {
                if (!jitCache.hasOwnProperty(pf.functionName)) {
                    const compiler = new Module.MathCompilerDP(pf.functionName, pf.name, pf.parameterName)
                    const fxn = compiler.compile(pf.stack);
                    updatePf({jitFunction: fxn})
                    jitCache[pf.functionName] = fxn;
                    compiler.delete()
                } else {
                    console.debug("cache hit for " + jitCache[pf.functionName])
                    updatePf({jitFunction: jitCache[pf.functionName]})
                }
                props.drawCallback(props.n, false)
                updatePf({drawing: false})
            }
        }

        // we don't automatically recompile (yet)
        // eslint-disable-next-line react-hooks/exhaustive-deps
    }, [pf.drawing])

    return (<div className="function-entry-container">
        <div className="function-control-panel">
            {pf.type === ProtofunctionType.C2C
                ? <Button minimal icon={pf.style.opacity === 0 ? "eye-off" : "eye-open"} onClick={event => {
                    if (pf.style.opacity === 0) {
                        updatePf({style: {...pf.style, opacity: 0.65}})
                    } else {
                        updatePf({style: {...pf.style, opacity: 0.}})
                    }
                }} intent={pf.style.opacity === 0 ? Intent.WARNING : Intent.NONE}/>
                : <button className="function-color-button" style={{backgroundColor: pf.style.stroke}} onClick={(event => {
                const newColor = `hsla(${Math.random() * 360}, 80%, 60%, 1)`;
                updatePf({style: {stroke: newColor}})
            })} />}

            <Button id="draw-function" minimal icon="play" disabled={!pf.stack || pf.stack.length === 0} loading={pf.drawing} onClick={event => {
                if (pf.jitFunction && pf.jitFunction.getName() === pf.functionName) {
                    return
                }
                updatePf({drawing: true})
            }} intent={pf.jitFunction ? (pf.jitFunction.getName() === pf.functionName ? Intent.SUCCESS : Intent.WARNING) : Intent.SUCCESS}/>

            <Button minimal icon="cog" disabled={!pf.jitFunction} onClick={event => setFunctionSourceOpen(true)} />
        </div>
{/*
        <input ref={inputRef} className="function-input-line" onInput={(event) => updatePf({text: (event.target as HTMLInputElement).value})}/>
        <div ref={prettyRef} className="function-pretty" />
*/}
        <MathQuillField className="function-input" onError={setError} onSuccess={(stack, fxnName, fxnArg, raw) => {
            setError(null)
            updatePf({
                name: fxnName,
                parameterName: fxnArg,
                text: raw,
                stack: stack,
                functionName: generateFunctionName(fxnName, fxnArg, stack)
            })
        }}/>
        {error === null ? <></> : <div className="function-error">{error}</div>}
        <div className="function-delete-button-container">
            <Button minimal small={true} icon="small-cross" intent={Intent.DANGER} onClick={() => {
                props.drawCallback(pf.id, true)
                if (pf.jitFunction) {
                    pf.jitFunction.release()
                }
                props.removeCallback()
            }} />
        </div>

        <InfoDialog isOpen={functionSourceOpen} closeCallback={() => setFunctionSourceOpen(false)} pfId={props.n} />
    </div>)
}

interface NewFunctionEntryProps {
    addFunction: (type: ProtofunctionType) => void
}

export const NewFunctionEntry: React.FC<NewFunctionEntryProps> = (props) => {

    return <div className="new-function-button-container">
            <Button id="new-point" minimal icon={NewPointIcon} intent={Intent.SUCCESS} large={true} onClick={() => props.addFunction(ProtofunctionType.C)} disabled={true}/>
            <Button id="new-arc" minimal icon={NewArcIcon} intent={Intent.SUCCESS} large={true} onClick={() => props.addFunction(ProtofunctionType.R2C)}/>
            <Button id="new-fxn" minimal intent={Intent.SUCCESS} large={true} onClick={() => props.addFunction(ProtofunctionType.C2C)}><b>&#x2102;</b></Button>
        </div>
}

