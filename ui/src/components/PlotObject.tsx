/*
 * Copyright 2021 Kioshi Morosin <glam@hex.lc>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

import React, {createRef, Dispatch, useContext, useEffect, useMemo, useReducer, useState} from "react";
import {Protofunction, ProtofunctionType, useProtofunction} from "../ProtofunctionContext";
import "./PlotObject.css"
import {SigArcFocus} from "../Signals";
import {SignalContext} from "../GlamContext";

export interface PlotObjectProps {
    pfId: number
    limits: [number, number, number, number]
    res: number
}

interface PlotObjectChildProps extends PlotObjectProps {
    pf: Protofunction
    updatePf: Dispatch<any>
}

export const PlotObject: React.FC<PlotObjectProps> = (props) => {
    const [pf, updatePf] = useProtofunction(props.pfId)

    switch (pf.type) {
        case ProtofunctionType.C:
            return <></>
        case ProtofunctionType.R2C:
            return <PlotArc {...props} pf={pf} updatePf={updatePf}/>
        case ProtofunctionType.C2C:
            return <ColorPlot {...props} pf={pf} updatePf={updatePf}/>
    }
}

export const PlotPoint: React.FC<PlotObjectChildProps> = (props) => {
    const [pf, updatePf] = useProtofunction(props.pfId)
    const point = useMemo(() => {
        if (pf.stack) {
            // do a one-off evaluation here
            return [0,0]
        }
    }, [pf.stack])

    return <circle id={`plot-point-${pf.id}`} className="plot-point" style={pf.style} r={0.1}/>
}

export const PlotArc: React.FC<PlotObjectChildProps> = (props) => {
    const multipoint = useMemo(() => {
        return new Module.RealMultipointDP(props.pf.jitFunction!, props.limits[0], props.limits[2], props.res)
    }, [props.pf.jitFunction?.getName() || ""])
    const [ctx, dispatch] = useContext(SignalContext)

    const points = useMemo(() => {
        if (multipoint) {
            console.debug("evaluating multipoint")
            multipoint.fullEval()
            return new Float64Array(multipoint.getValues())
        } else {
            // todo refine multipoint if needed
        }

    }, [multipoint])

    return (<polyline id={`plot-arc-${props.pfId}`} className="plot-arc" fill="none" style={props.pf.style}
                      points={points?.join(",")} onMouseOver={() => {
        dispatch(SigArcFocus({arc: props.pfId}))
    }} onMouseOut={() => dispatch(SigArcFocus({arc: -1}))}/>)
}

export const ColorPlot: React.FC<PlotObjectChildProps> = (props) => {
    const multipoint = useMemo(() => {
        return new Module.ComplexMultipointDP(props.pf.jitFunction!, [props.limits[0], props.limits[1]], [props.limits[2], props.limits[3]], props.res)
    }, [props.pf.jitFunction])

    const colors = useMemo(() => {
        if (multipoint) {
            console.debug("evaluating multipoint")
            multipoint.fullEval()
            const cb = multipoint.getColors()
            console.debug(cb)
            return cb
        } else {
            // todo refine multipoint if needed
        }

    }, [multipoint])

    const image = useMemo(() => {
        if (colors) {
            const canvas = document.createElement("canvas")
            const ctx = canvas.getContext("2d")
            if (!ctx) {
                console.error("could not get canvas context")
                return
            }
            const width = (props.limits[2] - props.limits[0]) * props.res
            const height = (props.limits[3] - props.limits[1]) * props.res
            canvas.width = width
            canvas.height = height
            console.debug("plot: " + width + "x" + height)

            const buffer = new Uint8ClampedArray(colors) // unfortunately we need to copy the color buffer here, but it's okay.
            const imageData = new ImageData(buffer, width)
            ctx.putImageData(imageData, 0, 0)
            return canvas.toDataURL("image/png")
        }
    }, [colors])

    useEffect(() => {
        props.updatePf({style: {...props.pf.style, opacity: 0.65}})
    }, [])

    return <image id={`color-plot-${props.pf.id}`} className="color-plot"
                  width={(props.limits[2] - props.limits[1])} height={(props.limits[3] - props.limits[1])} href={image}
                  x={props.limits[0]} y={props.limits[1]} preserveAspectRatio="xMidYMid meet"
                  style={props.pf.style}/>
}
