/**
 * @file sym.cpp
 * @author Sina Karvandi (sina@rayanfam.com)
 * @brief .sym command
 * @details
 * @version 0.1
 * @date 2021-05-27
 *
 * @copyright This project is released under the GNU Public License v3.
 *
 */
#include "..\hprdbgctrl\pch.h"

//
// Global Variables
//
extern BOOLEAN g_IsSerialConnectedToRemoteDebuggee;

/**
 * @brief help of .sym command
 *
 * @return VOID
 */
VOID
CommandSymHelp()
{
    ShowMessages(".sym : perfrom the symbol actions.\n\n");

    ShowMessages("syntax : \t.sym [table | reload | load | unload | download | add] [base (hex address)] "
                 "[path (string path to pdb)]\n");
    ShowMessages("\t\te.g : .sym table\n");
    ShowMessages("\t\te.g : .sym reload\n");
    ShowMessages("\t\te.g : .sym load\n");
    ShowMessages("\t\te.g : .sym download\n");
    ShowMessages("\t\te.g : .sym add base fffff8077356000 path c:\\symbols\\my_dll.pdb\n");
    ShowMessages("\t\te.g : .sym unload\n");
}

/**
 * @brief .sym command handler
 *
 * @param SplittedCommand
 * @param Command
 * @return VOID
 */
VOID
CommandSym(vector<string> SplittedCommand, string Command)
{
    UINT64 BaseAddress = NULL;
    string Delimiter   = "";
    string PathToPdb   = "";

    if (SplittedCommand.size() == 1)
    {
        ShowMessages("incorrect use of '.sym'\n\n");
        CommandSymHelp();
        return;
    }

    if (!SplittedCommand.at(1).compare("table"))
    {
        //
        // Validate params
        //
        if (SplittedCommand.size() != 2)
        {
            ShowMessages("incorrect use of '.sym'\n\n");
            CommandSymHelp();
            return;
        }

        if (g_IsSerialConnectedToRemoteDebuggee)
        {
            //
            // Just show the symbol table, not building it locally
            //
            SymbolBuildAndShowSymbolTable(FALSE);
        }
        else
        {
            //
            // Build locally and show symbol table
            //
            SymbolBuildAndShowSymbolTable(TRUE);
        }
    }
    else if (!SplittedCommand.at(1).compare("load") || !SplittedCommand.at(1).compare("download"))
    {
        //
        // Validate params
        //
        if (SplittedCommand.size() != 2)
        {
            ShowMessages("incorrect use of '.sym'\n\n");
            CommandSymHelp();
            return;
        }

        //
        // Load and download available symbols
        //
        if (!SplittedCommand.at(1).compare("load"))
        {
            SymbolLoadOrDownloadSymbols(FALSE, FALSE);
        }
        else if (!SplittedCommand.at(1).compare("download"))
        {
            SymbolLoadOrDownloadSymbols(TRUE, FALSE);
        }
    }
    else if (!SplittedCommand.at(1).compare("reload"))
    {
        //
        // Refresh and reload symbols
        //

        if (g_IsSerialConnectedToRemoteDebuggee)
        {
            //
            // Update symbol table from remote debuggee in debugger-mode
            //
            SymbolReloadSymbolTableInDebuggerMode();
        }
        else
        {
            //
            // Build locally and reload it
            //
            if (SymbolLocalReload())
            {
                ShowMessages("symbol table updated successfully\n");
            }
        }
    }
    else if (!SplittedCommand.at(1).compare("unload"))
    {
        //
        // Validate params
        //
        if (SplittedCommand.size() != 2)
        {
            ShowMessages("incorrect use of '.sym'\n\n");
            CommandSymHelp();
            return;
        }

        //
        // unload without any parameters, means that unload
        // all the symbols
        //
        ScriptEngineUnloadAllSymbolsWrapper();

        //
        // Size is 3 there is module name (not working ! I don't know why)
        //
        // ScriptEngineUnloadModuleSymbolWrapper((char *)SplittedCommand.at(2).c_str());
    }
    else if (!SplittedCommand.at(1).compare("add"))
    {
        //
        // Validate params
        //
        if (SplittedCommand.size() < 6)
        {
            ShowMessages("incorrect use of '.sym'\n\n");
            CommandSymHelp();
            return;
        }

        if (!SplittedCommand.at(2).compare("base"))
        {
            if (!ConvertStringToUInt64(SplittedCommand.at(3), &BaseAddress))
            {
                ShowMessages("please add a valid hex address to be used as the base address\n\n");
                CommandSymHelp();
                return;
            }

            //
            // Base address is now valid, check if next parameter is path
            //
            if (SplittedCommand.at(4).compare("path"))
            {
                ShowMessages("incorrect use of '.sym'\n\n");
                CommandSymHelp();
                return;
            }

            //
            // The rest of command is pdb path
            //
            Delimiter = "path ";
            PathToPdb = Command.substr(Command.find(Delimiter) + 5, Command.size());

            //
            // Check if pdb file exists or not
            //
            if (!IsFileExistA(PathToPdb.c_str()))
            {
                ShowMessages("pdb file not found\n");
                return;
            }

            ShowMessages("loading module symbol at '%s'\n", PathToPdb.c_str());

            //
            // Load the pdb file (the validation of pdb file is checked into pdb
            // parsing functions)
            //
            ScriptEngineLoadFileSymbolWrapper(BaseAddress, PathToPdb.c_str());
        }
        else
        {
            ShowMessages("incorrect use of '.sym'\n\n");
            CommandSymHelp();
            return;
        }
    }
    else
    {
        ShowMessages("unknown parameter at '%s'\n\n", SplittedCommand.at(1).c_str());
        CommandSymHelp();
        return;
    }
}
