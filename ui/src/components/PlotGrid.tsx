import React, {useContext, useEffect, useRef, useState} from "react";
import "./PlotGrid.css"
import {PlotObject} from "./PlotObject";

export interface PlotGridProps {
    majorTicks: number // math units between each major tick
    minorTicks: number // minor ticks per major tick
    limits: [number, number, number, number] // in math units
    drawList: number[]
}

export const PlotGrid: React.FC<PlotGridProps> = (props) => {
    const container = useRef<HTMLDivElement>(null);
    const [gridSteps, setGridSteps] = useState(0);
    const [originGridCoords, setOriginGridCoords] = useState<[number, number]>([0, 0]);
    const [nMajorTicks, setNMajorTicks] = useState([0, 0])

    useEffect(() => {
        if (!container.current) {
            return;
        }

        const viewportCenter = [container.current.offsetWidth / 2, container.current.offsetHeight / 2]

        const _gridSteps = Math.min(container.current.offsetWidth / (props.limits[1] - props.limits[0]), container.current.offsetHeight / (props.limits[3] - props.limits[2]));
        setNMajorTicks([Math.ceil(container.current.offsetWidth / _gridSteps), Math.ceil(container.current.offsetHeight / _gridSteps)])

        const _originGridCoords: [number, number] = [viewportCenter[0], viewportCenter[1]]
        setGridSteps(_gridSteps)
        setOriginGridCoords(_originGridCoords)

    }, [props])

    return (<div ref={container} className="plot-grid-container">
        <svg className="plot-space" width="100%" height="100%"
             viewBox={`${props.limits[0]} ${props.limits[1]} ${props.limits[2] - props.limits[0]} ${props.limits[3] - props.limits[1]}`}
             preserveAspectRatio="xMidYMid slice">
            <g className="coordinate-y-flip">
                <PlotAxes limits={props.limits} majorInterval={props.majorTicks} minorTicksPerMajor={props.minorTicks} hideMinorGrid={false} />
                {props.drawList.map(id => {
                    return <PlotObject key={id} pfId={id} limits={[props.limits[0], props.limits[1], props.limits[2], props.limits[3]]} res={50} />
                })}
            </g>
        </svg>
    </div>)
}

export interface PlotAxisProps {
    limits: [number, number, number, number]
    majorInterval: number
    minorTicksPerMajor: number
    hideMinorGrid: boolean
}

const PlotAxes: React.FC<PlotAxisProps> = (props) => {
    const minorInterval = props.majorInterval / props.minorTicksPerMajor;

    return (<>
            <defs>
                <pattern id="grid-lines-minor" width={minorInterval}
                         height={minorInterval}
                         patternUnits="userSpaceOnUse">
                    <path className="plot-grid-minor-line" d={`M ${minorInterval} 0 L 0 0 0 ${minorInterval}`}/>
                </pattern>
                <pattern id="grid-lines-major" width={props.majorInterval} height={props.majorInterval} patternUnits="userSpaceOnUse">
                    {props.hideMinorGrid ? <></> : <rect width={props.majorInterval} height={props.majorInterval} fill="url(#grid-lines-minor)"/>}
                    <path className="plot-grid-major-line" d={`M ${props.majorInterval} 0 L 0 0 0 ${props.majorInterval}`}/>
                </pattern>
            </defs>

            <rect x={props.limits[0]} y={props.limits[1]} width="100%" height="100%" fill="url(#grid-lines-major)" />

            <line className="plot-grid-axis-line" x1={props.limits[0]} y1="0" x2={props.limits[2]} y2="0" />
            <line className="plot-grid-axis-line" x1="0" y1={props.limits[1]} x2="0" y2={props.limits[3]} />

            {[...Array(Math.ceil(props.limits[2] / props.majorInterval)).keys()].map(n => n+1).map(n => {
                return <text key={n + ",0"} className="plot-grid-major-label" x={n} y="0" fontSize="0.1">{n}</text>
            })}

            {[...Array(Math.ceil(Math.abs(props.limits[0] / props.majorInterval))).keys()].map(n => n+1).map(n => {
                return <text key={"-" + n + ",0"} className="plot-grid-major-label" x={-n} y="0" fontSize="0.1">-{n}</text>
            })}


            {[...Array(Math.ceil(props.limits[3] / props.majorInterval)).keys()].map(n => n+1).map(n => {
                return <text key={"0," + n}className="plot-grid-major-label" x="0" y={n} fontSize="0.1">-{n}i</text>
            })}

            {[...Array(Math.ceil(Math.abs(props.limits[1] / props.majorInterval))).keys()].map(n => n+1).map(n => {
                return <text key={"0,-" + n} className="plot-grid-major-label" x="0" y={-n} fontSize="0.1">{n}i</text>
            })}
    </>)
}
