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

enum MLNodeType {
    NUMBER,
    IDENTIFIER,
    PAREN,
    BINARY,
    FUNCTION_APPLY,
    ROOT,
}

function simpleTypeFromNode(node: Node): MLNodeType {
    switch (node.nodeName) {
        case "mn": return MLNodeType.NUMBER
        case "mi": return MLNodeType.IDENTIFIER
        case "mo": return MLNodeType.BINARY
    }
    throw "illegal node"
}

export class MLNode {
    parent?: MLNode
    xmlName: string
    type: MLNodeType
    contents?: string
    children: MLNode[] = []

    constructor(xmlName: string, type: MLNodeType, parent?: MLNode) {
        this.xmlName = xmlName
        this.type = type
        this.parent = parent
    }
}

export class MathMLLexer {
    private node: Node
    private functionName?: string
    private functionParam?: string

    constructor(node: Node) {
        if (node.nodeName !== "mrow") {
            throw "not an mrow node!"
        }
        this.node = node
    }

    static isMRow(node: Node) { return node.nodeName === "mrow" }
    static isMN(node: Node) { return node.nodeName === "mn" }
    static isMI(node: Node) { return node.nodeName === "mi" }
    static isMO(node: Node) { return node.nodeName === "mo" }
    static isParenRow(node: Node) {
        return this.isMRow(node) && node.childNodes.length >= 2
            && node.childNodes[0].textContent === "(" && node.childNodes[node.childNodes.length - 1].textContent === ")"
    }


    visitMRow(node: Node): MLNode {
        const paren = new MLNode("mrow", MLNodeType.PAREN)
        paren.children = [...node.childNodes].map(child => this.visitNode(child, paren)).filter(v => v !== undefined) as MLNode[]
        return paren
    }

    visitLeaf(node: Node, parent?: MLNode): MLNode | undefined {
        const x = new MLNode(node.nodeName, simpleTypeFromNode(node), parent)
        // parentheses are handled by the mrow tags surrounding them
        if (node.textContent === "(" || node.textContent === ")") return undefined
        if (parent && node.nodeName === "mo" && node.textContent === "\u2061") {
            // when a unary operator like sin is used, we get a \u2061 character as our operator between the
            // actual operator and its argument
            return new MLNode("mo", MLNodeType.FUNCTION_APPLY, parent)
        }
        x.contents = node.textContent || undefined
        return x
    }

    visitNode(node: Node, parent?: MLNode): MLNode | undefined {
        if (MathMLLexer.isMRow(node)) {
            return this.visitMRow(node)
        }

        if (MathMLLexer.isMN(node) || MathMLLexer.isMI(node) || MathMLLexer.isMO(node)) {
            return this.visitLeaf(node, parent)
        }

        if (node.nodeName === "msub") {
            if (MathMLLexer.isMI(node.childNodes[0])) {
                const x = new MLNode("", MLNodeType.IDENTIFIER)
                x.contents = node.childNodes[0]! + "_" + node.childNodes[1]!
                return x
            }
        }

        if (node.nodeName === "msup") {
            const base = this.visitNode(node.childNodes[0])
            const exp = this.visitNode(node.childNodes[1])
            const op = new MLNode("", MLNodeType.BINARY)
            op.contents = "^"
            if (base && exp) {
                const p = new MLNode("", MLNodeType.PAREN)
                p.children.push(base, op, exp)
                return p
            }
        }

        if (node.nodeName === "mfrac") {
            const num = this.visitNode(node.childNodes[0])
            const den = this.visitNode(node.childNodes[1])
            const op = new MLNode("", MLNodeType.BINARY)
            op.contents = "/"
            if (num && den) {
                const p = new MLNode("", MLNodeType.PAREN)
                p.children.push(num, op, den)
                return p
            }
        }

        return
    }

    visitMath(): MLNode {
        return this.visitMRow(this.node)
    }
}

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

export class MathMLParser {
    stack: StackObject[] = []
    parseError?: string

    private isExpression(node: MLNode) {
        return node.type === MLNodeType.NUMBER || (node.type === MLNodeType.IDENTIFIER && node.contents?.length === 1) || node.type === MLNodeType.PAREN
    }

    visitChildren(node: MLNode) {
        if (node.children.length === 0) return
        if (node.children.length === 1) {
            this.visitNode(node.children[0])
            return
        }

        for (let i = 0; i < node.children.length; ) {
            const n = node.children[i]
            const next = node.children[i+1]

            if (next) {
                if (n.contents && next.type === MLNodeType.FUNCTION_APPLY && i + 2 < node.children.length) {
                    this.visitNode(node.children[i + 2])
                    this.stack.push({type: StackObjectType.OPERATOR, value: n.contents})
                    i += 3
                    continue
                }
            }

            if (this.isExpression(n)) {
                if (next.type === MLNodeType.BINARY) {
                    i++;
                    continue;
                }

                if (n.type === MLNodeType.NUMBER && next.type === MLNodeType.IDENTIFIER && next.contents === "i") {
                    this.stack.push({type: StackObjectType.NUMBER, value: n.contents + "i"})
                    i += 2
                    continue
                }

                if (this.isExpression(next)) {
                    this.visitNode(n)
                    let j
                    for (j = i + 1; j < node.children.length && this.isExpression(node.children[j]); j++) {
                        this.visitNode(node.children[j])
                        this.stack.push({type: StackObjectType.OPERATOR, value: "\\cdot"})
                    }
                    i = j + 1
                    continue
                }
            }


            if (n.type === MLNodeType.BINARY && n.contents) {
                if (i + 1 >= node.children.length) {
                    this.error("incomplete binary expression")
                }
                const prev = node.children[i-1]
                const next = node.children[i+1]

                if (next && this.isExpression(next)) {
                    if (!prev && n.contents === "\u2212") {
                        this.visitNode(next)
                        this.stack.push({type: StackObjectType.NUMBER, value: "-1"})
                        this.stack.push({type: StackObjectType.OPERATOR, value: "\\cdot"})
                        i += 2
                        continue
                    }
                    if (prev && this.isExpression(prev)) {
                        this.visitNode(prev)
                        this.visitNode(next)
                        this.stack.push({type: StackObjectType.OPERATOR, value: n.contents})
                        i += 2
                        continue
                    }
                }
            }

            this.visitChildren(n)
            i++
        }
    }

    visitNode(node: MLNode) {
        switch (node.type) {
            case MLNodeType.NUMBER:
                this.stack.push({type: StackObjectType.NUMBER, value: node.contents!})
                break
            case MLNodeType.IDENTIFIER:
                this.stack.push({type: StackObjectType.IDENTIFIER, value: node.contents!})
                break
            default:
                this.visitChildren(node)
        }
    }

    private error(msg: string) {
        this.parseError = msg
        this.stack = []
    }
}