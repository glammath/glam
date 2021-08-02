import AsciiMathParser from "./AsciiMathParser";
import {ProtofunctionType} from "./ProtofunctionContext";
import sha1 from "crypto-js/sha1";

interface AMExpression {
    tex: string
    pos: number
    end: number
    ttype: string
    exprs?: AMExpression[]
    op?: AMExactExpression
    arg?: AMExpression
    dash?: {token: string, ttype: string}
}

interface AMBracketExpression extends AMExpression {
    left: AMExpression
    middle: AMExpression
    right: AMExpression
}

interface AMBinaryExpression extends AMExpression {
    arg1: AMExpression
    arg2: AMExpression
    op: AMExactExpression
}

interface AMExactExpression extends AMExpression {
    token: string
}

interface AMSubSupExpression extends AMExpression {
    sub?: AMExpression
    sup?: AMExpression
}

interface AMIntermediateExpression extends AMExpression {
    expression: AMExpression
    subsup?: AMSubSupExpression // or something else?
}

interface AMFractionExpression extends AMExpression {
    numerator: AMExpression
    denominator: AMExpression
}

enum StackObjectType {
    NUMBER,
    IDENTIFIER,
    OPERATOR
}

export interface StackObject {
    value: string
    type: StackObjectType
}

export interface ExpressionVisitor {
    visit(expr: AMExpression): void
}

export class FunctionDeclarationVisitor implements ExpressionVisitor {
    parser: ExpressionParser

    constructor(_parser: ExpressionParser) {
        this.parser = _parser

    }

    visit(expr: AMExpression): AMExpression | null {
        if (this.parser.functionName || this.parser.functionArg) {
            this.parser.abort("duplicate function definition")
            return null
        }

        if (expr.ttype.startsWith("expression") && expr.exprs && expr.exprs.length === 1) {
            return this.visit(expr.exprs[0])
        }

        if (!expr.exprs || expr.exprs.length < 2) {
            this.parser.abort("empty equation")
            return null
        }

        if (expr.exprs![0].ttype === "unary") { // special-cased for `f` and `g` for some reason
            const unaryExpr = expr.exprs![0]
            const rhsExpr = expr.exprs![1]
            if (unaryExpr.arg && unaryExpr.arg.ttype === "bracket") {
                if (rhsExpr.exprs && rhsExpr.exprs.length === 2 && rhsExpr.exprs[0].tex === "=") {
                    this.parser.functionArg = (unaryExpr.arg as AMBracketExpression).middle.tex
                    this.parser.functionName = unaryExpr.op!.token // unary type should guarantee this exists
                    return rhsExpr.exprs[1]
                }
            }
        } else if (expr.ttype === "expression") {
            if (expr.exprs && expr.exprs.length === 2) {
                const nameExpr = expr.exprs[0]
                const rightExpr = expr.exprs[1]
                if (nameExpr.ttype === "literal" || nameExpr.ttype === "greek") {
                    if (rightExpr.exprs && rightExpr.exprs.length === 2) {
                        const argExpr = rightExpr.exprs[0]
                        const rhsExpr = rightExpr.exprs[1]
                        if (argExpr.ttype === "bracket" && rhsExpr.exprs && rhsExpr.exprs.length === 2 && rhsExpr.exprs[0].tex === "=") {
                            this.parser.functionName = nameExpr.tex
                            this.parser.functionArg = (argExpr as AMBracketExpression).middle.tex
                            return rhsExpr.exprs[1]
                        }
                    }
                } else {
                    this.parser.abort("illegal function name")
                    return null
                }
            }
        }
        this.parser.abort("unable to find function declaration")
        return null
    }
}

export class FunctionDefinitionVisitor implements ExpressionVisitor {
    parser: ExpressionParser
    parent: ExpressionVisitor
    amParser = new AsciiMathParser()

    constructor(_parser: ExpressionParser, _parent: ExpressionVisitor) {
        this.parser = _parser
        this.parent = _parent
    }

    visit(expr: AMExpression): void {
        if (expr.ttype.startsWith("expression") && expr.exprs) {
            if (expr.exprs.length === 1) {
                this.visit(expr.exprs[0])
                return
            } else if (expr.exprs.length === 2) {
                if (expr.exprs[0].ttype === "arbitrary_constant" || expr.exprs[0].ttype === "other_constant") {
                    this.visit(expr.exprs[1])
                    this.visitObject({type: StackObjectType.OPERATOR, value: expr.exprs[0].tex}) // todo this is wrong
                    return
                }
            }
        }

        if (expr.dash) {
            this.visit((expr as AMIntermediateExpression).expression)
            this.visitObject({type: StackObjectType.OPERATOR, value: "-"})
            return
        }

        if (expr.ttype === "fraction" || (expr.ttype === "binary" && expr.op && expr.op.tex === "\\frac")) {
            const fractionExpr = expr as AMFractionExpression
            if (fractionExpr.numerator && fractionExpr.denominator) {
                this.visitFractionPart(fractionExpr.numerator)
                this.visitFractionPart(fractionExpr.denominator)
                this.visitObject({type: StackObjectType.OPERATOR, value: "/"})
            } else {
                const binaryExpr = expr as AMBinaryExpression
                this.visitFractionPart(binaryExpr.arg1)
                this.visitFractionPart(binaryExpr.arg2)
                this.visitObject({type: StackObjectType.OPERATOR, value: "/"})
            }
            return
        }

        if ((expr.ttype === "literal" || expr.ttype === "greek") && (!expr.exprs || expr.exprs.length === 0)) {
            // we're at the end of a branch
            this.visitLiteral(expr.tex)
            return
        }

        if (expr.ttype === "intermediate") {
            const intermediate = expr as AMIntermediateExpression
            this.visitSubsup(intermediate)
            return
        }

        if (expr.ttype === "bracket") {
            const bracketExpr = expr as AMBracketExpression
            if (!bracketExpr.left || !bracketExpr.right || !bracketExpr.middle) {
                this.parser.abort("incomplete bracket expression")
                return
            }

            if (bracketExpr.left.tex === "(" && bracketExpr.right.tex === ")") {
                this.visit(bracketExpr.middle)
                return
            } else {
                this.parser.abort("unrecognized bracket type")
                return
            }
        }

        if (expr.exprs && expr.exprs.length === 2) {
            this.visitBranch(expr)
            return
        }

        if (expr.ttype === "unary") {
            this.visitUnaryOperation(expr)
            return
        }
    }

    visitBranch(expr: AMExpression) {
        if (!expr.exprs || expr.exprs.length < 2) {
            this.parser.abort("not a branch expression")
            return
        }

        const leftExpr = expr.exprs[0]
        const rightExpr = expr.exprs[1]

        if (leftExpr.ttype === "literal" || leftExpr.ttype === "intermediate" || leftExpr.ttype === "bracket" || leftExpr.ttype === "fraction" || leftExpr.ttype === "greek") {
            if (rightExpr.ttype === "expression") {
                if (rightExpr.exprs![1].tex.startsWith("+") || rightExpr.exprs![1].tex.startsWith("-")) {
                    this.visitBinaryOperation(leftExpr, "\\cdot", rightExpr.exprs![0])
                    this.visit(rightExpr.exprs![1])
                    return
                }
            } else if (rightExpr.ttype === "negative_expression") {
                this.visitBinaryOperation(leftExpr, "-", (rightExpr as AMIntermediateExpression).expression)
                return
            }

            if ((leftExpr.ttype === "literal" || leftExpr.ttype === "greek") && rightExpr.tex === "i") {
                if (isNaN(parseFloat(leftExpr.tex))) {
                    this.visitObject({type: StackObjectType.IDENTIFIER, value: leftExpr.tex})
                    this.visitLiteral("i")
                    this.visitObject({type: StackObjectType.OPERATOR, value: "\\cdot"})
                } else {
                    this.visitLiteral(leftExpr.tex)
                    this.visitLiteral("i")
                    this.visitObject({type: StackObjectType.OPERATOR, value: "\\cdot"})
                }
                return
            }
            if (rightExpr.exprs && rightExpr.exprs.length === 2) {
                if (rightExpr.exprs[0].ttype === "arbitrary_constant") {
                    this.visitBinaryOperation(leftExpr, rightExpr.exprs[0].tex, rightExpr.exprs[1])
                    return
                }
            }

            // two terms adjacent for implied multiplication
            this.visitBinaryOperation(leftExpr, "\\cdot", rightExpr)
            return
        }
    }

    visitBinaryOperation(leftExpr: AMExpression, morpheme: string, rightExpr: AMExpression) {
        this.visit(leftExpr)
        this.visit(rightExpr)
        this.visitObject({type: StackObjectType.OPERATOR, value: morpheme})
    }

    visitUnaryOperation(expr: AMExpression) {
        if (!expr.arg || !expr.op) {
            this.parser.abort("not a unary operation")
            return
        }
        this.visit(expr.arg)
        this.visitObject({type: StackObjectType.OPERATOR, value: expr.op.token})
    }

    visitLiteral(literal: string) {
        if (isNaN(parseFloat(literal))) {
            this.visitObject({type: StackObjectType.IDENTIFIER, value: literal})
        } else {
            this.visitObject({type: StackObjectType.NUMBER, value: literal})
        }
    }

    visitSubsup(expr: AMIntermediateExpression) {
        if (!expr.subsup) {
            this.parser.abort("not a subscript/superscript expression")
            return
        }

        // todo check for integrals and sums here

        if (expr.subsup.sup) { // exponentiation
            this.visit(expr.expression)

            const exponent = expr.subsup.sup.tex.replace("{", "").replace("}", "") // we need to parse this again
            this.amParser.input(exponent)
            const parseResult = this.amParser.consume() as AMExpression
            if (parseResult && parseResult.exprs && parseResult.exprs[0]) {
                this.visit(parseResult.exprs[0])
                this.visitObject({type: StackObjectType.OPERATOR, value: "^"})
            } else {
                this.parser.abort("incomplete superscript")
                return
            }
        } else if (expr.subsup.sub) {
            if (expr.expression.ttype === "literal") {
                this.visitLiteral(expr.expression.tex + "_" + expr.subsup.sub!.tex)
                return
            } else {
                this.parser.abort("illegal subscript")
                return
            }
        }
    }

    visitFractionPart(expr: AMExpression) {
        if (!expr) {
            this.parser.abort("incomplete fraction")
            return
        }
        if (expr.ttype && expr.ttype === "literal") {
            this.visit(expr)
        } else {
            this.amParser.input(expr.tex)
            const parseResult = this.amParser.consume()
            if (parseResult && parseResult.exprs && parseResult.exprs[0]) {
                this.visit(parseResult.exprs[0])
            } else {
                this.parser.abort("incomplete fraction")
                return
            }
        }

    }

    visitObject(object: StackObject) {
        this.parser.stack.push(object)
    }
}

export class ExpressionParser implements ExpressionVisitor {
    functionName?: string
    functionArg?: string
    stack: StackObject[] = []
    error?: string
    type?: ProtofunctionType = ProtofunctionType.R2C

    visit(expr: AMExpression): void {
        const declarationVisitor = new FunctionDeclarationVisitor(this)
        const definitionExpr = declarationVisitor.visit(expr)
        const definitionVisitor = new FunctionDefinitionVisitor(this, this)
        if (definitionExpr) {
            definitionVisitor.visit(definitionExpr)
        }
    }

    abort(error: string) {
        this.stack = []
        this.error = error
    }

    generateFunctionName() {
        const raw = this.type + "_" + this.functionName + "_" + this.functionArg + this.stack.map(obj => obj.type + ":" + obj.value).join("_")
        return "__jit_" + sha1(raw).toString().substr(0, 16)
    }
}
