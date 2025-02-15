/**
 * @file x.cpp
 * @author Sina Karvandi (sina@rayanfam.com)
 * @brief parse symbols
 * @details
 * @version 0.1
 * @date 2021-05-31
 *
 * @copyright This project is released under the GNU Public License v3.
 *
 */
#include "..\hprdbgctrl\pch.h"

/**
 * @brief help of x command
 *
 * @return VOID
 */
VOID
CommandXHelp()
{
    ShowMessages("x : search and show symbols (wildcard) and corresponding addresses.\n\n");
    ShowMessages("syntax : \tx [module!name (string)]\n");
    ShowMessages("\t\te.g : x nt!ExAllocatePoolWithTag \n");
    ShowMessages("\t\te.g : x nt!ExAllocatePool* \n");
    ShowMessages("\t\te.g : x nt!* \n");
}

/**
 * @brief x command handler
 *
 * @param SplittedCommand
 * @param Command
 * @return VOID
 */
VOID
CommandX(vector<string> SplittedCommand, string Command)
{
    if (SplittedCommand.size() == 1)
    {
        ShowMessages("incorrect use of 'x'\n\n");
        CommandXHelp();
        return;
    }

    //
    // Trim the command
    //
    Trim(Command);

    //
    // Remove x from it
    //
    Command.erase(0, 1);

    //
    // Trim it again
    //
    Trim(Command);

    //
    // Search for mask
    //
    ScriptEngineSearchSymbolForMaskWrapper(Command.c_str());
}
