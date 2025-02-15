/**
 * @file script-engine.h
 * @author Sina Karvandi (sina@rayanfam.com)
 * @brief General script-engine functions and wrappers
 * @details
 * @version 0.1
 * @date 2021-09-23
 *
 * @copyright This project is released under the GNU Public License v3.
 *
 */
#pragma once

//////////////////////////////////////////////////
//    Pdb Parser Wrapper (from script-engine)   //
//////////////////////////////////////////////////
UINT64
ScriptEngineConvertNameToAddressWrapper(const char * FunctionOrVariableName, PBOOLEAN WasFound);

UINT32
ScriptEngineLoadFileSymbolWrapper(UINT64 BaseAddress, const char * PdbFileName);

VOID
ScriptEngineSetTextMessageCallbackWrapper(PVOID Handler);

UINT32
ScriptEngineUnloadAllSymbolsWrapper();

UINT32
ScriptEngineUnloadModuleSymbolWrapper(char * ModuleName);

UINT32
ScriptEngineSearchSymbolForMaskWrapper(const char * SearchMask);

BOOLEAN
ScriptEngineGetFieldOffsetWrapper(CHAR * TypeName, CHAR * FieldName, DWORD32 * FieldOffset);

BOOLEAN
ScriptEngineCreateSymbolTableForDisassemblerWrapper(void * CallbackFunction);

BOOLEAN
ScriptEngineConvertFileToPdbPathWrapper(const char * LocalFilePath, char * ResultPath);

BOOLEAN
ScriptEngineConvertFileToPdbFileAndGuidAndAgeDetailsWrapper(const char * LocalFilePath,
                                                            char *       PdbFilePath,
                                                            char *       GuidAndAgeDetails);

BOOLEAN
ScriptEngineSymbolInitLoadWrapper(PMODULE_SYMBOL_DETAIL BufferToStoreDetails,
                                  UINT32                StoredLength,
                                  BOOLEAN               DownloadIfAvailable,
                                  const char *          SymbolPath,
                                  BOOLEAN               IsSilentLoad);

VOID
ScriptEngineSymbolAbortLoadingWrapper();

//////////////////////////////////////////////////
//          Script Engine Wrapper               //
//////////////////////////////////////////////////

VOID
ScriptEngineWrapperTestParser(string Expr);

BOOLEAN
ScriptAutomaticStatementsTestWrapper(string Expr, UINT64 ExpectationValue, BOOLEAN ExceptError);

PVOID
ScriptEngineParseWrapper(char * Expr, BOOLEAN ShowErrorMessageIfAny);

VOID
PrintSymbolBufferWrapper(PVOID SymbolBuffer);

UINT64
ScriptEngineWrapperGetHead(PVOID SymbolBuffer);

UINT32
ScriptEngineWrapperGetSize(PVOID SymbolBuffer);

UINT32
ScriptEngineWrapperGetPointer(PVOID SymbolBuffer);

VOID
ScriptEngineWrapperRemoveSymbolBuffer(PVOID SymbolBuffer);

UINT64
ScriptEngineEvalUInt64StyleExpressionWrapper(string Expr, PBOOLEAN HasError);

//////////////////////////////////////////////////
//          Script Engine Functions             //
//////////////////////////////////////////////////

UINT64
ScriptEngineEvalSingleExpression(string Expr, PBOOLEAN HasError);
