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

import React, {useEffect, useRef, useState} from "react";
import {MathField, MathFieldConfig} from "../MathQuill";

import "./MathQuillRestyler.css"
import sha1 from "crypto-js/sha1";

interface MathQuillFieldProps {
    config?: MathFieldConfig
    onError?: (msg: string) => void
    onSuccess?: (stack: StackObject[], name: string, arg: string, raw: string) => void
    className?: string
}

const mq = MathQuill.getInterface(2)

enum StackObjectType {
    NUMBER,
    IDENTIFIER,
    OPERATOR,
    FXNCALL
}

export interface StackObject {
    value: string
    type: StackObjectType
}

export class SemanticParser {
    static UNARY_OPS = [ // order here matters
        "sinh",
        "cosh",
        "tanh",
        "sech",
        "csch",
        "coth",
        "sin",
        "cos",
        "tan",
        "sec",
        "csc",
        "cot",
        "arg"
    ]

    fxnName?: string
    fxnParam?: string
    private symbol = ""
    private skipTo = -1
    stack: StackObject[] = []
    private tokens: RegExpMatchArray[] = []
    parseErr: string | null = null

    // matches a function definition in the form `f(z)=z+1` with the first capture group giving the function name,
    // the second giving the parameter name, and the third giving the string representing the definition.
    private static FUNCTION_PATTERN = /^([a-zA-Z\\]+(?:_[a-zA-Z0-9,]+)?)\*? ?\(([a-zA-Z\\]+(?:_[a-zA-Z0-9,\\]+)?)\) ?= ?(.*)$/

    // matches tokens in a function definition, capturing the following groups:
    // 1 - decimal numbers, i.e. 2.5. will also capture something like 2.5*i
    // 2 - multi-character identifiers (which MQ puts a space after). may be a variable or a unary operator
    // 3 - single-character identifiers
    // 4 - binary operators, one of +-*/^
    // 5 - ( or )
    private static TOKENIZER = /(-?\d+(?:\.\d+)?)(?:\*i)?|([a-zA-Z0-9]{2,} (?:_\([^)]+\)|_[a-zA-Z0-9])?)|([a-zA-Z](?:_\([^)]+\)|_[a-zA-Z0-9])?)|([+\-*/^])|([()])/g

    parseError(err: string) {
        this.parseErr = err
    }

    visitNumber(num: string) {
        this.stack.push({type: StackObjectType.NUMBER, value: num})
    }

    visitIdentifier(id: string) {
        if (id === this.fxnParam || Module.Globals.isGlobal(id)) {
            this.stack.push({type: StackObjectType.IDENTIFIER, value: id})
        } else {
            this.parseError("unrecognized symbol")
        }
    }

    visitBinary(op: string) {
        this.stack.push({type: StackObjectType.OPERATOR, value: op})
    }

    visitUnary(op: string) {
        this.stack.push({type: StackObjectType.OPERATOR, value: op})
    }

    maybeVisitMultisymbol(i: number): boolean {
        if (this.symbol !== "") {
            const thisToken = this.tokens[i]

            if (thisToken[3]) {
                this.symbol += thisToken[3]
            }

            const op = SemanticParser.UNARY_OPS.find(op => this.symbol.startsWith(op))
            if (!op) {
                this.parseError("illegal unary operator")
                return false
            }
            const rem = this.symbol.substr(op.length)
            this.symbol = ""
            if (rem && rem.length > 0) {
                this.visitIdentifier(rem)
            } else {
                this.visitToken(i + 1)
            }
            this.visitUnary(op)
            return true
        }
        return false
    }

    visitToken(i: number) {
        if (i >= this.tokens.length) {
            return
        }

        const token = this.tokens[i]
        const prev = this.tokens[i - 1]
        const next = this.tokens[i + 1]

        if (token[5]) {
            if (token[5] === ")") {
                this.skipTo = i + 1
                return
            } else if (token[5] === "(") {
                this.visitToken(i + 1)
            } else {
                this.parseError("malformed bracket expression")
                return
            }
        } else if (token[1]) {
            const overnext = this.tokens[i + 2]
            if (next && next[4] && (next[4] === "+" || next[4] === "-") && overnext && overnext[1] && overnext[1].endsWith("i")) {
                this.visitNumber(token[1] + next[4] + overnext[1])
                this.visitToken(i + 2)
                return
            } else {
                this.visitNumber(token[1])
                this.visitToken(i + 1)
                return;
            }
            return
        } else if (token[2]) {
            // we need to check if it smashed a unary operator with another identifier
            // e.g. sin Gamma  will be exported as sinGamma
        } else if (token[3]) {
            if (next && next[3]) {
                this.symbol += token[3]
            } else {
                if (!this.maybeVisitMultisymbol(i)) {
                    this.visitIdentifier(token[3])
                }
            }
        } else if (token[4]) {
            if (token[4] === "-") {
                // check for unary negative operator
                if (next && (next[3] || next[5] || next[1]) && (!prev || !prev[3] || !prev[1] || prev[5] === "(")) {
                    this.visitNumber("-1")
                    this.visitToken(i + 1)
                    this.visitBinary("*")
                    return
                }
            }

            if (!prev || !next) {
                this.parseError("incomplete binary operation")
                return;
            }
            this.visitToken(i + 1)
            this.visitBinary(token[4])
            return
        }

        if (this.skipTo > 0) {
            if (this.tokens[this.skipTo]) {
                const x = this.skipTo
                this.skipTo = -1
                this.visitToken(x)
                return
            } else {
                return
            }
        } else {
            this.visitToken(i + 1)
        }
    }

    visitMath(text: string) {
        this.tokens = [...text.matchAll(SemanticParser.TOKENIZER)]
        this.visitToken(0)
    }

    visitFunctionDefinition(text: string) {
        const match = text.match(SemanticParser.FUNCTION_PATTERN)
        if (!match || match.length !== 4) {
            this.parseError("malformed function definition")
            return
        }
        this.fxnName = match[1]
        this.fxnParam = match[2]
        this.visitMath(match[3])
    }
}

export function generateFunctionName(fxnName: string, fxnParam: string, stack: StackObject[]) {
    const raw = fxnName + "_" + fxnParam + stack.map(obj => obj.type + ":" + obj.value).join("_")
    return "__jit_" + sha1(raw).toString().substr(0, 16)
}

export const MathQuillField: React.FC<MathQuillFieldProps> = (props) => {
    const [mf, setMf] = useState<MathField>()
    const divRef = useRef<HTMLDivElement>(null)

    useEffect(() => {
        if (divRef.current) {
            const field = mq.MathField(divRef.current, {
                ...props.config,
                handlers: {
                    edit: field => {
                        const text = field.__controller.exportText()
                        const parser = new SemanticParser()
                        parser.visitFunctionDefinition(text)

                        if (parser.parseErr && props.onError) {
                            props.onError(parser.parseErr)
                        } else if (props.onSuccess && parser.fxnName && parser.fxnParam) {
                            props.onSuccess(parser.stack, parser.fxnName, parser.fxnParam, text)
                        }

                        if (props.config?.handlers?.edit) {
                            props.config.handlers.edit(field)
                        }
                    }
                }
            })
            field.el().classList.add("mq-restyler-field")
            field.focus()

            setMf(field)
        }
    }, [divRef, props.config])

    return <div className={props.className} ref={divRef} />
}
