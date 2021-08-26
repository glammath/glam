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

export enum Direction {
    LEFT = -1,
    RIGHT = 1
}

export type TraversalTarget<T> = Record<Direction, T>

export interface TreeTraversal<T> {
    ends: TraversalTarget<T>
}

export interface TreeNode extends TreeTraversal<TreeNode | 0>, TraversalTarget<TreeNode | 0> {
    id: number
}

export interface TreeLiteralNode extends TreeNode {
    ctrlSeq: string
    htmlTemplate: string
}

export interface TreeSubNode extends TreeNode {
    sub: TreeTraversal<TreeNode | 0>
}

export interface TreeSupNode extends TreeNode {
    sup: TreeTraversal<TreeNode | 0>
}

export interface TreeBlockNode extends TreeNode {
    blocks: TreeTraversal<TreeNode | 0>[]
    side: number
    sides: TraversalTarget<{ch: string, ctrlSeq: string}>
}

interface MathFieldController {
    root: TreeTraversal<TreeNode | 0>
    exportText(): string
    exportLatex(): string
}

export interface MathField {
    __controller: MathFieldController
    revert(): HTMLElement
    reflow(): void
    el(): HTMLElement
    latex(): string
    latex(str: string): void
    focus(): void
    blur(): void
    write(str: string): void
    cmd(c: string): void
    select(): void
    clearSelection(): void
    moveToLeftEnd(): void
    moveToRightEnd(): void
    keystroke(key: string): void
    config(c: MathFieldConfig): void
}

export interface MathFieldConfig {
    spaceBehavesLikeTab?: boolean
    leftRightIntoCmdGoes?: string
    restrictMismatchedBrackets?: boolean
    sumStartsWithNEquals?: boolean
    supSubsRequireOperand?: boolean
    charsThatBreakOutOfSupSub?: string
    autoSubscriptNumerals?: boolean
    autoCommands?: string
    autoOperatorNames?: string
    maxDepth?: number
    substituteTextArea?: () => HTMLElement
    handlers?: {
        edit?: (field: MathField) => void
        upOutOf?: (field: MathField) => void
        moveOutOf?: (field: MathField) => void
    }
}

export interface MQ {
    MathField(element: HTMLElement, config?: MathFieldConfig): MathField
}

declare global {
    export class MathQuill {
        static getInterface(ver: number): MQ

    }
}
