using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace SSIDE.Classes
{
    public class IDEOptions
    {
        public bool BracketMatching;
        public bool ShowScopeIndicator;
        public bool ShowWhitespace;
        public bool ShowEOLMarker;
        public bool ShowGutterMargin;
        public bool ShowLineNumbers;
        public bool EnableAutoComplete;
        public bool AutoCompleteSimpleExpressions;
        public bool AutoCompleteComplexExpressions;
        public bool AutoCompleteMatchOnBeginningOnly;
        public bool DebugShowConsole;
        public bool EnableCodeFolding;
        public string DefaultText;
        public int DefaultCaretX;
        public int DefaultCaretY;

        public IDEOptions()
        {
            LoadFromRegistry();
        }

        public void SaveToRegistry()
        {
            global.SetRegistryBool("IDE\\Options", "ShowLineNumbers", ShowLineNumbers);
            global.SetRegistryBool("IDE\\Options", "ShowGutterMargin", ShowGutterMargin);
            global.SetRegistryBool("IDE\\Options", "ShowEOLMarker", ShowEOLMarker);
            global.SetRegistryBool("IDE\\Options", "ShowWhitespace", ShowWhitespace);
            global.SetRegistryBool("IDE\\Options", "ShowScopeIndicator", ShowScopeIndicator);
            global.SetRegistryBool("IDE\\Options", "BracketMatching", BracketMatching);
            global.SetRegistryBool("IDE\\Options", "EnableAutoComplete", EnableAutoComplete);
            global.SetRegistryBool("IDE\\Options", "AutoCompleteSimpleExpressions", AutoCompleteSimpleExpressions);
            global.SetRegistryBool("IDE\\Options", "AutoCompleteComplexExpressions", AutoCompleteComplexExpressions);
            global.SetRegistryBool("IDE\\Options", "AutoCompleteMatchOnBeginningOnly", AutoCompleteMatchOnBeginningOnly);
            global.SetRegistryBool("IDE\\Options", "DebugShowConsole", DebugShowConsole);
            global.SetRegistryBool("IDE\\Options", "EnableCodeFolding", EnableCodeFolding);
            global.SetRegistryString("IDE\\Options", "DefaultText", DefaultText);
            global.SetRegistryInt("IDE\\Options", "DefaultCaretX", DefaultCaretX);
            global.SetRegistryInt("IDE\\Options", "DefaultCaretY", DefaultCaretY);
        }

        public void LoadFromRegistry()
        {
            ShowLineNumbers = global.GetRegistryBool("IDE\\Options", "ShowLineNumbers", true);
            ShowGutterMargin = global.GetRegistryBool("IDE\\Options", "ShowGutterMargin", true);
            ShowEOLMarker = global.GetRegistryBool("IDE\\Options", "ShowEOLMarker", false);
            ShowWhitespace = global.GetRegistryBool("IDE\\Options", "ShowWhitespace", false);
            ShowScopeIndicator = global.GetRegistryBool("IDE\\Options", "ShowScopeIndicator", true);
            BracketMatching = global.GetRegistryBool("IDE\\Options", "BracketMatching", true);
            EnableAutoComplete = global.GetRegistryBool("IDE\\Options", "EnableAutoComplete", true);
            AutoCompleteSimpleExpressions = global.GetRegistryBool("IDE\\Options", "AutoCompleteSimpleExpressions", true);
            AutoCompleteComplexExpressions = global.GetRegistryBool("IDE\\Options", "AutoCompleteComplexExpressions", true);
            AutoCompleteMatchOnBeginningOnly = global.GetRegistryBool("IDE\\Options", "AutoCompleteMatchOnBeginningOnly");
            DebugShowConsole = global.GetRegistryBool("IDE\\Options", "DebugShowConsole");
            EnableCodeFolding = global.GetRegistryBool("IDE\\Options", "EnableCodeFolding");
            DefaultText = global.GetRegistryString("IDE\\Options", "DefaultText", "");
            DefaultCaretX = global.GetRegistryInt("IDE\\Options", "DefaultCaretX", 0);
            DefaultCaretY = global.GetRegistryInt("IDE\\Options", "DefaultCaretY", 0);
        }
    }
}
