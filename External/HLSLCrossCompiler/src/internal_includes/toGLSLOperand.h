#ifndef TO_GLSL_OPERAND_H
#define TO_GLSL_OPERAND_H

#include "internal_includes/structs.h"

#define TO_FLAG_NONE    0x0
#define TO_FLAG_INTEGER 0x1
#define TO_FLAG_NAME_ONLY 0x2
#define TO_FLAG_DECLARATION_NAME 0x4
#define TO_FLAG_DESTINATION 0x8 //Operand is being written to by assignment.
#define TO_FLAG_UNSIGNED_INTEGER 0x10
#define TO_FLAG_DOUBLE 0x20
void TranslateOperand(HLSLCrossCompilerContext* psContext, const Operand* psOperand, uint32_t ui32TOFlag);

int GetMaxComponentFromComponentMask(const Operand* psOperand);
void TranslateOperandIndex(HLSLCrossCompilerContext* psContext, const Operand* psOperand, int index);
void TranslateOperandIndexMAD(HLSLCrossCompilerContext* psContext, const Operand* psOperand, int index, uint32_t multiply, uint32_t add);
void TranslateOperandSwizzle(HLSLCrossCompilerContext* psContext, const Operand* psOperand);
uint32_t GetNumSwizzleElements(const Operand* psOperand);
void AddSwizzleUsingElementCount(HLSLCrossCompilerContext* psContext, uint32_t count);
int GetFirstOperandSwizzle(HLSLCrossCompilerContext* psContext, const Operand* psOperand);
uint32_t IsSwizzleReplacated(const Operand* psOperand);

void TextureName(HLSLCrossCompilerContext* psContext, const uint32_t ui32RegisterNumber, const int bZCompare);

//Non-zero means the components overlap
int CompareOperandSwizzles(const Operand* psOperandA, const Operand* psOperandB);

SHADER_VARIABLE_TYPE GetOperandDataType(HLSLCrossCompilerContext* psContext, const Operand* psOperand);

#endif
