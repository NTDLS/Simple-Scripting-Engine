using System;
using System.Collections.Generic;
using System.Windows.Forms;
using SSIDE.Classes;
using Newtonsoft.Json;
using System.Linq;

namespace SSIDE.Forms
{
    public partial class frmMain
    {
        AutoCompleteItems autoCompleteList = null;
        public AutoCompleteItems AutoCompleteList
        {
            get
            {
                if (autoCompleteList == null)
                {
                    string helpPath = global.GetRegistryString("", "Path", null);
                    if (helpPath != null)
                    {
                        string autoCompleteFile = helpPath + "\\Help\\AutoComplete.json";
                        string jsonText = System.IO.File.ReadAllText(autoCompleteFile);
                        autoCompleteList = JsonConvert.DeserializeObject<AutoCompleteItems>(jsonText);
                    }
                }

                return autoCompleteList;
            }
        }

        string GetDocumentationText(string keyword)
        {
            try
            {
                keyword = keyword.ToLower();
                List<AutoCompleteItem> tips = (from o in AutoCompleteList where o.Keyword == keyword select o).ToList();

                List<string> tipTexts = new List<string>();

                foreach (var tip in tips)
                {
                    if (tip.Type != string.Empty && tip.Type != null)
                    {
                        tipTexts.Add(string.Format("{0}.{1}: {2}", tip.Type, tip.Topic, tip.Content));
                    }
                    else if (tip.Topic != string.Empty && tip.Topic != null)
                    {
                        tipTexts.Add(string.Format("{0}: {1}", tip.Topic, tip.Content));
                    }
                }

                if (tipTexts.Count > 0)
                {
                }

                return string.Join("\r\n\r\n", tipTexts.OrderBy(o => o).ToArray());
            }
            catch//(Exception ex)
            {
            }

            return null;
        }

        public class AutoCompleteWord
        {
            private string _Text;
            private int _Image;

            public AutoCompleteWord(string text, int image)
            {
                _Text = text;
                _Image = image;
            }

            public string Text
            {
                get { return _Text; }
                set { _Text = value; }
            }

            public int Image
            {
                get { return _Image; }
                set { _Image = value; }
            }
        }

        void CodeEditor_KeyUp(object sender, KeyEventArgs e)
        {
            if (_IDEOptions.EnableAutoComplete)
            {
                if (((CodeEditor)sender).AllowAutoComplete)
                {
                    TriggerEditorAutoComplete((NTDLS.Windows.Forms.CodeEditorControl)sender, e);
                }
            }
        }

        void TriggerEditorAutoComplete(NTDLS.Windows.Forms.CodeEditorControl editor, KeyEventArgs e)
        {
            //We only auto-insert on tab and enter. This allows users to type words that are not in the list (aka variable names).

            if (e.Control || e.Alt || e.KeyCode == Keys.ShiftKey)
            {
                return; //Close on "control keys".
            }

            if (e.KeyCode == Keys.OemPeriod)
            {
                editor.AutoListClear(); //Reset the auto-list on period press.
            }

            if (!((e.KeyValue >= 'A' && e.KeyValue <= 'Z')  //A-Z
                || (e.KeyValue > '0' && e.KeyValue < '9')   //0-9
                || e.KeyCode == Keys.OemPeriod              //.
                || (e.KeyValue == '3' && e.Shift == true)   //# (for pre-processors)
                ))
            {
                if (e.KeyCode == Keys.Up || e.KeyCode == Keys.PageUp || e.KeyCode == Keys.PageDown || e.KeyCode == Keys.Down)
                {
                    return; //Allow user to scroll thorugh list.
                }

                editor.AutoListVisible = false; //Close on all invalid keys.
                return;
            }

            //If the auto-list has already been opened, then we will not repopulate it.
            if (editor.AutoListVisible && editor.AutoListItems.Count > 0)
            {
                return;
            }

            //Only open the auto-list if an alpha key (or period) were pressed.
            if (!((e.KeyValue >= 'A' && e.KeyValue <= 'Z')  //A-Z
                || e.KeyCode == Keys.OemPeriod              //.
                || (e.KeyValue == '3' && e.Shift == true)   //# (for pre-processors)
                ))
            {
                return;
            }

            NTDLS.Syntax.Segment currentSegment = editor.Caret.CurrentSegment();
            if (currentSegment.BlockType.Name == "Text")
            {
                return; //We do not provide auto-completion within the text area.
            }

            //Save the caret positions.
            int caretX = editor.Caret.Position.X;
            int caretY = editor.Caret.Position.Y;
            if (caretX < 0 || caretY < 0)
            {
                return;
            }

            //Get the current word.
            NTDLS.Syntax.TextPoint currentWordPos = new NTDLS.Syntax.TextPoint(caretX - 1, caretY);
            NTDLS.Syntax.Word currentWord = editor.Document.GetValidWordFromPos(currentWordPos);
            if (currentWord == null || currentWord.Text.Length <= 0)
            {
                editor.AutoListVisible = false;
                return;
            }

            //We do not perform auto-listing within strings.
            if (currentWord.Style.Name == "Strings Style" || currentWord.Style.Name == "Comments Style")
            {
                editor.AutoListVisible = false;
                return; //We do not provide auto-completion within strings or comments.
            }

            //Subtract the current word length from the CaretX, this is so that
            //  the X position points to the beginning of the word on the document.
            caretX -= currentWord.Text.Length;
            if (caretX < 0 || caretY < 0)
            {
                return;
            }
            string currentWordText = currentWord.Text;

            //If the current word is a period, then get the word before it.
            if (currentWordText == ".")
            {
                caretX--;

                //Get the current word.
                currentWordPos = new NTDLS.Syntax.TextPoint(caretX - 1, caretY);
                currentWord = editor.Document.GetValidWordFromPos(currentWordPos);
                if (currentWord == null || currentWord.Text.Length <= 0)
                {
                    editor.AutoListVisible = false;
                    return;
                }

                caretX = currentWord.Column;
                currentWordText = currentWord.Text + '.';
            }
            editor.AutoListClear();
            editor.AutoListBeginLoad();

            List<AutoCompleteWord> listWords = new List<AutoCompleteWord>();

            foreach (NTDLS.Syntax.PatternList patternList in currentSegment.BlockType.KeywordsList)
            {
                if (patternList.Name != "Numbers")
                {
                    int imageSegment = 10;
                    int imageTerminator = 5;

                    if (patternList.Name == "Types")
                    {
                        imageSegment = 10;
                        imageTerminator = 8;
                    }
                    else if (patternList.Name == "Constants")
                    {
                        imageSegment = 10;
                        imageTerminator = 5;
                    }
                    else if (patternList.Name == "System")
                    {
                        imageSegment = 10;
                        imageTerminator = 6;
                    }
                    else if (patternList.Name == "Keywords")
                    {
                        imageSegment = 10;
                        imageTerminator = 6;
                    }
                    else if (patternList.Name == "Reserved")
                    {
                        imageSegment = 10;
                        imageTerminator = 9;
                    }
                    else if (patternList.Name == "PreProcessors")
                    {
                        imageSegment = 10;
                        imageTerminator = 11;
                    }

                    if (_IDEOptions.AutoCompleteSimpleExpressions)
                    {
                        foreach (System.Collections.DictionaryEntry pattern in patternList.SimplePatterns)
                        {
                            string patternText = pattern.Key.ToString();

                            if (_IDEOptions.AutoCompleteMatchOnBeginningOnly)
                            {
                                if (patternText.StartsWith(currentWordText, StringComparison.CurrentCultureIgnoreCase))
                                {
                                    GetAutocompleteWords(listWords, patternText, imageSegment, imageTerminator);
                                }
                            }
                            else
                            {
                                if (patternText.ToLower().Contains(currentWordText.ToLower()))
                                {
                                    GetAutocompleteWords(listWords, patternText, imageSegment, imageTerminator);
                                }
                            }
                        }
                    }

                    if (_IDEOptions.AutoCompleteComplexExpressions)
                    {
                        foreach (NTDLS.Syntax.Pattern pattern in patternList.ComplexPatterns)
                        {
                            if (_IDEOptions.AutoCompleteMatchOnBeginningOnly)
                            {
                                if (pattern.StringPattern.StartsWith(currentWordText, StringComparison.CurrentCultureIgnoreCase))
                                {
                                    GetAutocompleteWords(listWords, pattern.StringPattern, imageSegment, imageTerminator);
                                }
                            }
                            else
                            {
                                if (pattern.StringPattern.ToLower().Contains(currentWordText.ToLower()))
                                {
                                    GetAutocompleteWords(listWords, pattern.StringPattern, imageSegment, imageTerminator);
                                }
                            }
                        }
                    }
                }
            }

            //Add the words to the visual drop-down-list.
            foreach (AutoCompleteWord word in listWords)
            {
                string tipText = GetDocumentationText(word.Text);
                if (tipText == null)
                {
                    //tipText = "No documentation is available for [" + word.Text + "]";
                }

                editor.AutoListAdd(word.Text, word.Text, tipText, word.Image);
            }

            editor.AutoListEndLoad();

            if (editor.AutoListItems.Count > 0)
            {
                NTDLS.Syntax.TextPoint autoListPostion = new NTDLS.Syntax.TextPoint(caretX, caretY);

                editor.AutoListPosition = autoListPostion;
                editor.AutoListAutoSelect = true;
                editor.AutoListVisible = true;
            }
            else
            {
                editor.AutoListVisible = false;
            }
        }

        /// <summary>
        /// Splits a fully qualified dot-denoted function or variable name into its various sub parts.
        /// </summary>
        /// <param name="qualifiedWord"></param>
        /// <returns></returns>
        void GetAutocompleteWords(List<AutoCompleteWord> words, string qualifiedWord, int imageSegment, int imageTerminator)
        {
            int length = qualifiedWord.Length;
            int image = 0;

            for (int index = 0; index < length; index++)
            {
                index = qualifiedWord.IndexOf('.', index);
                if (index < 0)
                {
                    image = imageTerminator;
                    index = length;
                }
                else
                {
                    image = imageSegment;
                }

                string word = qualifiedWord.Substring(0, index);

                //Only add distinct items to the list.
                if (words.Find(n => n.Text == word && n.Image == image) == null)
                {
                    words.Add(new AutoCompleteWord(word, image));
                }
            }
        }
    }
}
