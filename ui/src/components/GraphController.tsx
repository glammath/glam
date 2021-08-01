import {GlamAppContextProvider} from "../GlamContext";
import {ProtofunctionContextProvider} from "../ProtofunctionContext";
import {LeftSidebar} from "./LeftSidebar";
import {PlotGrid} from "./PlotGrid";
import React, {useCallback, useState} from "react";

export const GraphController: React.FC = (props) => {
    const [drawList, setDrawList] = useState<number[]>([])
    const draw = useCallback((pfId: number, remove: boolean = false) => {
        if (remove) {
            setDrawList(drawList.filter(n => n !== pfId))
        } else if (!drawList.includes(pfId)) {
            setDrawList([...drawList, pfId])
        }
    }, [drawList, setDrawList])

    return (<>
        <GlamAppContextProvider>
            <ProtofunctionContextProvider>
                <LeftSidebar drawCallback={draw} />
                <PlotGrid majorTicks={1} minorTicks={5} limits={[-4, -4, 4, 4]} drawList={drawList}/>
            </ProtofunctionContextProvider>
        </GlamAppContextProvider>
    </>)
}