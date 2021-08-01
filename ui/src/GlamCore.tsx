import {StackObject} from "./ExpressionParser";
import React from "react";

export type ptr = number
export type f64 = number
export type i32 = number
export type u32 = number
export type complex = [number, number]

export interface Multipoint<T> {
    new(func: Fxn, from: T, to: T, res: u32): Multipoint<T>

    fullEval(): void
    getValues(): Float64Array
    getColors(): Float64Array
    delete(): void
}

export interface Fxn {
    ready(): boolean
    release(): void
    getName(): string
    getFxnName(): string
    getParameterName(): string
    getDisassembly(): string
}

export interface MathCompilerDP {
    new(name: string, fxnName: string, parameterName: string): MathCompilerDP
    compile(stack: StackObject[]): Fxn
}

export interface GlamCoreModule extends EmscriptenModule {
    MathCompilerDP: MathCompilerDP
    RealMultipointMP: Multipoint<number>
    ComplexMultipointMP: Multipoint<complex>
    RealMultipointDP: Multipoint<number>
    ComplexMultipointDP: Multipoint<complex>
    ccall: typeof ccall
}

declare global {
    const wasmMemory: WebAssembly.Memory
    const wasmTable: WebAssembly.Table
    const Module: GlamCoreModule
}

export function f64BufferView(address: ptr, len: i32) {
    return Module.HEAPF64.subarray(address / 8, (address / 8) + len)
}
