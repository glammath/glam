import {Classes, Dialog, Label, Slider, Tab, Tabs} from "@blueprintjs/core";
import React, {useMemo} from "react";
import {useProtofunction} from "../ProtofunctionContext";
import "./InfoDialog.css"

interface InfoDialogProps {
    isOpen: boolean
    closeCallback: () => void;
    pfId: number
}

export const InfoDialog: React.FC<InfoDialogProps> = (props) => {
    const [pf, updatePf] = useProtofunction(props.pfId)
    const disassembly = useMemo(() => {
        if (pf.jitFunction) {
            return pf.jitFunction.getDisassembly()
        } else {
            return "module is not compiled!"
        }
    }, [pf.jitFunction])

    return <Dialog className="bp3-dark info-dialog" title={`${pf.id}: ${pf.name}(${pf.parameterName})`} isOpen={props.isOpen} onClose={props.closeCallback}>
        <Tabs id="InfoTabs" className={Classes.DIALOG_BODY} vertical={true}>
            <Tab id="style" title="Style" panel={<>
                <Label>
                    Opacity
                    <p />
                    <Slider min={0.0} max={1.0} stepSize={0.005} vertical={true}
                            onChange={n => updatePf({style: {...pf.style, opacity: n}})}
                            value={parseFloat((pf.style.opacity || 1).toString())}/>
                </Label>
            </>} />
            <Tab id="disassembly" title="Disassembly" panel={<div className={Classes.CODE_BLOCK} >
                    <pre>{disassembly}</pre>
                </div>}/>
        </Tabs>
    </Dialog>
}