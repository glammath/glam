import {Protofunction} from "./ProtofunctionContext";
import {complex} from "./GlamCore";

export interface Signal {
    signalName: string
    _dataType: (t: any) => void
}

export type SS<T> = (data: T) => Signal

function _build<T>(name: string, extra = {}): SS<T> {
    return (data: T) => {
        return {
            signalName: name,
            ...data,
            ...extra,
            _dataType: (t: T) => {}
        }
    };
}

export const SigReady: SS<{}> = _build("Ready")
export const SigArcFocus: SS<{arc: number}> = _build("ArcFocus")
export const SigTreeRecalculate: SS<{root: string, value: complex}> = _build("TreeRecalculate")
