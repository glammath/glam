import React, {useCallback, useContext, useEffect, useState} from "react";
import "./FunctionEntry"
import {FunctionEntry, NewFunctionEntry} from "./FunctionEntry";
import "./LeftSidebar.css"
import AsciiMathParser from "../AsciiMathParser";
import {ProtofunctionType} from "../ProtofunctionContext";

interface SidebarProps {
    drawCallback: (id: number, remove: boolean) => void
}

export const LeftSidebar: React.FC<SidebarProps> = (props) => {
    const parser = new AsciiMathParser()
    const [nextPf, setNextPf] = useState(0)
    const [activePfs, setActivePfs] = useState<Map<number, ProtofunctionType>>(new Map())

    return <div className="left-sidebar">
        <p>there will be something here eventually i promise</p>
        {activePfs.size > 0
            ? [...activePfs].map(pair => <FunctionEntry key={pair[0]} n={pair[0]} removeCallback={() => {
                setActivePfs(pfs => {
                    const newState = new Map(pfs)
                    newState.delete(pair[0])
                    return newState
                })
            }} parser={parser} drawCallback={props.drawCallback} type={pair[1]}/>)
            : <></>}
        <NewFunctionEntry addFunction={(type => {
            setActivePfs(oldState => {
                const newState = new Map(oldState)
                newState.set(nextPf, type)
                return newState
            })
            setNextPf(n => n + 1)
        })}/>
        </div>
}