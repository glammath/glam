import React, {useContext, useEffect, useState} from "react";
import {SignalContext} from "../GlamContext";
import "./MouseTooltip.css"

interface MouseTooltipProps {
    grid: HTMLDivElement
    mathToGridCoords: (mathCoords: [number, number]) => [number, number]
    gridToMathCoords: (gridCoords: [number, number]) => [number, number]
}

export const MouseTooltip: React.FC<MouseTooltipProps> = (props) => {
    const [visible, setVisible] = useState(false)
    const [[x, y], setMousePos] = useState([0, 0])
    const [ctx] = useContext(SignalContext)

    useEffect(() => {
        props.grid.addEventListener("mousemove", (event: MouseEvent) => {
            setMousePos([event.offsetX, event.offsetY])
        })
    }, [props.grid])

    return (<div className="mouse-tooltip" style={{visibility: visible ? "visible" : "hidden", top: y, left: x}}>
        <p className="coordinate">{props.gridToMathCoords([x, y])}</p>
        {ctx.focusedArc !== 0 ? <p className="focused-arc">{ctx.focusedArc}</p>: <></>}
    </div>)
}