import React, {useCallback, useContext, useEffect, useState} from "react";
import "./FunctionEntry"
import {FunctionEntry, NewFunctionEntry} from "./FunctionEntry";
import "./LeftSidebar.css"
import {ProtofunctionType} from "../ProtofunctionContext";
import {AnchorButton} from "@blueprintjs/core";

interface SidebarProps {
    drawCallback: (id: number, remove: boolean) => void
}

export const LeftSidebar: React.FC<SidebarProps> = (props) => {
    const [nextPf, setNextPf] = useState(0)
    const [activePfs, setActivePfs] = useState<Map<number, ProtofunctionType>>(new Map())

    return <div className="left-sidebar">
        <div style={{padding: "2px" }}>
            <AnchorButton text={"Github"} href={"https://github.com/glammath/glam"} target={"_blank"}/>
            <span style={{paddingLeft: "25%", font: "14pt bold"}}>Glam</span>
        </div>
        {activePfs.size > 0
            ? [...activePfs].map(pair => <FunctionEntry key={pair[0]} n={pair[0]} removeCallback={() => {
                setActivePfs(pfs => {
                    const newState = new Map(pfs)
                    newState.delete(pair[0])
                    return newState
                })
            }} drawCallback={props.drawCallback} type={pair[1]}/>)
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
