using System;
using System.Windows.Forms;

namespace SSIDE.Forms
{
    public partial class frmMain
    {
        public delegate void deAddImmediateInfo(String sValue);
        public delegate void deImmediateAutoListBegin();
        public delegate void deImmediateAutoListEnd();
        public delegate void deImmediateAutoListAddWord(String sType, String sName);

        void Immediate_MouseDown(object sender, MouseEventArgs e)
        {
            ((NTDLS.Windows.Forms.CodeEditorControl)sender).AutoListVisible = false;
        }

        void Immediate_Leave(object sender, EventArgs e)
        {
            ((NTDLS.Windows.Forms.CodeEditorControl)sender).AutoListVisible = false;
        }

        public void AddImmediateInfo(String sValue)
        {
            _ImmediateText.Document.Text = _ImmediateText.Document.Text + "\r\n" + sValue + "\r\n";
            _ImmediateText.Caret.MoveAbsoluteEnd(false);
        }

        private void _ImmediateText_KeyDown(object sender, KeyEventArgs e)
        {
            if (e.KeyCode == Keys.Enter)
            {
                e.SuppressKeyPress = true;

                NTDLS.Windows.Forms.CodeEditorControl editor = (NTDLS.Windows.Forms.CodeEditorControl)sender;

                string rowText = editor.Caret.CurrentRow.Text;

                if (rowText.Length > 0)
                {
                    if (_RunningApplication.IsRunning)
                    {
                        WriteToCmdPipe("::Immediate~|" + rowText);
                    }
                    else
                    {
                        AddImmediateInfo("<Not Running>");
                    }
                }
            }
        }

        void ImmediateAutoListBegin()
        {
            _ImmediateText.AutoListClear();
            _ImmediateText.AutoListBeginLoad();
        }

        void ImmediateAutoListEnd()
        {
            _ImmediateText.AutoListEndLoad();

            if (_ImmediateText.Caret != null)
            {
                if (_ImmediateText.Caret.CurrentWord != null)
                {
                    string currentWord = _ImmediateText.Caret.CurrentWord.Text;

                    if (currentWord.Length > 0)
                    {
                        if (_ImmediateText.AutoListItems.Count > 0)
                        {
                            //int caretX = _ImmediateText.Caret.Position.X - currentWord.Length;
                            //int caretY = _ImmediateText.Caret.Position.Y;

                            //NTDLS.Syntax.TextPoint autoListPostion = new NTDLS.Syntax.TextPoint(caretX, caretY);

                            //_ImmediateText.AutoListPosition = autoListPostion;
                            _ImmediateText.AutoListAutoSelect = true;
                            _ImmediateText.AutoListVisible = true;
                        }
                    }
                }
            }
        }

        void ImmediateAutoListAddWord(String sType, String sName)
        {
            if (sType == "Var")
            {
                _ImmediateText.AutoListAdd(sName, 5);
            }
            else if (sType == "GenericVar")
            {
                _ImmediateText.AutoListAdd(sName, 3);
            }
            else if (sType == "ScriptProc")
            {
                _ImmediateText.AutoListAdd(sName, 8);
            }
            else if (sType == "SysProc")
            {
                _ImmediateText.AutoListAdd(sName, 6);
            }
        }

        private void _ImmediateText_KeyUp(object sender, KeyEventArgs e)
        {
            if (_IDEOptions.EnableAutoComplete)
            {
                TriggerImmediateAutoComplete(_ImmediateText, e);
            }
        }

        string ParseImmediateAutoCompleteWord(string sWord)
        {
            if (sWord.Length > 0)
            {
                int highestIndex = 0;

                char[] separators = ",+-*^\\/()[]{}@:;'?£$#%& \t=<>".ToCharArray();
                foreach (char seperator in separators)
                {
                    int index = sWord.LastIndexOf(seperator);
                    if (index > highestIndex)
                    {
                        highestIndex = index;
                    }
                }

                if (highestIndex > 0)
                {
                    return sWord.Substring(highestIndex + 1);
                }
            }

            return sWord;
        }

        void TriggerImmediateAutoComplete(NTDLS.Windows.Forms.CodeEditorControl editor, KeyEventArgs e)
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

            if (!((e.KeyValue >= 'A' && e.KeyValue <= 'Z') || (e.KeyValue > '0' && e.KeyValue < '9') || e.KeyCode == Keys.OemPeriod))
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
            if (!((e.KeyValue >= 'A' && e.KeyValue <= 'Z') || e.KeyCode == Keys.OemPeriod))
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
            string currentWordText = ParseImmediateAutoCompleteWord(currentWord.Text);

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

            //string currentWord = editor.Caret.CurrentWord.Text;
            if (_RunningApplication.IsRunning)
            {
                if (_ImmediateText.Caret != null)
                {
                    if (currentWordText.Length > 0)
                    {
                        caretX = _ImmediateText.Caret.Position.X - currentWordText.Length;
                        caretY = _ImmediateText.Caret.Position.Y;

                        NTDLS.Syntax.TextPoint autoListPostion = new NTDLS.Syntax.TextPoint(caretX, caretY);

                        _ImmediateText.AutoListPosition = autoListPostion;
                        WriteToCmdPipe("::ImmediateAutoList~|" + currentWordText);
                    }
                }
            }
        }
    }
}
