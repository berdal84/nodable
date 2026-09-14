#pragma once

#include <string>
#include <vector>
#include <array>
#include <memory>
#include <unordered_set>
#include <unordered_map>
#include <map>
#include <regex>
#include "ndbl/gui/geometry/Vec2.h" // this header should be in the core folder..

namespace ndbl
{
	
struct Text_Editor
{
	enum class PaletteIndex
	{
		Default,
		Keyword,
		Number,
		String,
		CharLiteral,
		Punctuation,
		Preprocessor,
		Identifier,
		KnownIdentifier,
		PreprocIdentifier,
		Comment,
		MultiLineComment,
		Background,
		Cursor,
		Selection,
		ErrorMarker,
		Breakpoint,
		LineNumber,
		CurrentLineFill,
		CurrentLineFillInactive,
		CurrentLineEdge,
		Max
	};

	enum class SelectionMode
	{
		Normal,
		Word,
		Line
	};

	struct Breakpoint
	{
		int mLine;
		bool mEnabled;
		std::string mCondition;

		Breakpoint()
			: mLine(-1)
			, mEnabled(false)
		{}
	};

	// Represents a character coordinate from the user's point of view,
	// i. e. consider an uniform grid (assuming fixed-width font) on the
	// screen as it is rendered, and each cell has its own coordinate, starting from 0.
	// Tabs are counted as [1..mTabSize] count empty spaces, depending on
	// how many space is necessary to reach the next tab stop.
	// For example, coordinate (1, 5) represents the character 'B' in a line "\tABC", when mTabSize = 4,
	// because it is rendered as "    ABC" on the screen.
	struct Coordinates
	{
		int mLine, mColumn;
		Coordinates() : mLine(0), mColumn(0) {}
		Coordinates(int aLine, int aColumn) : mLine(aLine), mColumn(aColumn)
		{
			assert(aLine >= 0);
			assert(aColumn >= 0);
		}
		static Coordinates Invalid() { static Coordinates invalid(-1, -1); return invalid; }

		bool operator ==(const Coordinates& o) const
		{
			return
				mLine == o.mLine &&
				mColumn == o.mColumn;
		}

		bool operator !=(const Coordinates& o) const
		{
			return
				mLine != o.mLine ||
				mColumn != o.mColumn;
		}

		bool operator <(const Coordinates& o) const
		{
			if (mLine != o.mLine)
				return mLine < o.mLine;
			return mColumn < o.mColumn;
		}

		bool operator >(const Coordinates& o) const
		{
			if (mLine != o.mLine)
				return mLine > o.mLine;
			return mColumn > o.mColumn;
		}

		bool operator <=(const Coordinates& o) const
		{
			if (mLine != o.mLine)
				return mLine < o.mLine;
			return mColumn <= o.mColumn;
		}

		bool operator >=(const Coordinates& o) const
		{
			if (mLine != o.mLine)
				return mLine > o.mLine;
			return mColumn >= o.mColumn;
		}
	};

	struct Identifier
	{
		Coordinates mLocation;
		std::string mDeclaration;
	};

	typedef std::string String;
	typedef std::unordered_map<std::string, Identifier> Identifiers;
	typedef std::unordered_set<std::string> Keywords;
	typedef std::map<int, std::string> ErrorMarkers;
	typedef std::unordered_set<int> Breakpoints;
	typedef uint8_t Char;

	struct Glyph
	{
		Char mChar;
		PaletteIndex mColorIndex = PaletteIndex::Default;
		bool mComment : 1;
		bool mMultiLineComment : 1;
		bool mPreprocessor : 1;

		Glyph(Char aChar, PaletteIndex aColorIndex) : mChar(aChar), mColorIndex(aColorIndex),
			mComment(false), mMultiLineComment(false), mPreprocessor(false) {}
	};

	typedef std::vector<Glyph> Line;
	typedef std::vector<Line> Lines;

	struct LanguageDefinition
	{
		typedef std::pair<std::string, PaletteIndex> TokenRegexString;
		typedef std::vector<TokenRegexString> TokenRegexStrings;
		typedef bool(*TokenizeCallback)(const char * in_begin, const char * in_end, const char *& out_begin, const char *& out_end, PaletteIndex & paletteIndex);

		std::string mName;
		Keywords mKeywords;
		Identifiers mIdentifiers;
		Identifiers mPreprocIdentifiers;
		std::string mCommentStart, mCommentEnd, mSingleLineComment;
		char mPreprocChar;
		bool mAutoIndentation;

		TokenizeCallback mTokenize;

		TokenRegexStrings mTokenRegexStrings;

		bool mCaseSensitive;

		LanguageDefinition()
			: mPreprocChar('#'), mAutoIndentation(true), mTokenize(nullptr), mCaseSensitive(true)
		{
		}

		static const LanguageDefinition& CPlusPlus();
		static const LanguageDefinition& HLSL();
		static const LanguageDefinition& GLSL();
		static const LanguageDefinition& C();
		static const LanguageDefinition& SQL();
		static const LanguageDefinition& AngelScript();
		static const LanguageDefinition& Lua();
	};

	struct EditorState
	{
		Coordinates mSelectionStart;
		Coordinates mSelectionEnd;
		Coordinates mCursorPosition;
	};

	struct UndoRecord
	{
		void Undo(Text_Editor* aEditor);
		void Redo(Text_Editor* aEditor);

		std::string mAdded;
		Coordinates mAddedStart;
		Coordinates mAddedEnd;

		std::string mRemoved;
		Coordinates mRemovedStart;
		Coordinates mRemovedEnd;

		EditorState mBefore;
		EditorState mAfter;
	};
	typedef std::vector<UndoRecord> UndoBuffer;
	typedef void (*Add_Undo_Handler)(Text_Editor&, UndoRecord&);
	typedef void (*Set_Clipboard_Handler)(const char*);
	typedef const char* (*Get_Clipboard_Handler)();
	typedef std::vector<std::pair<std::regex, PaletteIndex>> RegexList;


	static void 		DefaultAddUndoHandler(Text_Editor& editor, UndoRecord& value);
	static void 		DefaultSetClipBoardText(const char* str);
	static const char*  DefaultGetClipBoardText();

	float 				mLineSpacing;
	Lines 				mLines;
	EditorState 		mState;
	UndoBuffer 			mUndoBuffer;
	int 				mUndoIndex;
	Add_Undo_Handler    mAddUndoHandler;
	Get_Clipboard_Handler get_clipboard_text_proc;
	Set_Clipboard_Handler set_clipboard_text_proc;

	int 				mTabSize;
	bool 				mOverwrite;
	bool 				mReadOnly;
	bool 				mWithinRender;
	bool 				mScrollToCursor;
	bool 				mScrollToTop;
	bool 				mTextChanged;
	bool 				mColorizerEnabled;
	float 				mTextStart; // position (in pixels) where a code line starts relative to the left of the Text_Editor.
	int  				mLeftMargin;
	bool 				mCursorPositionChanged;
	int 				mColorRangeMin, mColorRangeMax;
	SelectionMode 		mSelectionMode;
	bool 				mHandleKeyboardInputs;
	bool 				mHandleMouseInputs;
	// bool 			mIgnoreImGuiChild; moved to xxx_ImGui_Impl.cpp
	bool 				mShowWhitespaces;

	LanguageDefinition 	mLanguageDefinition;
	RegexList 			mRegexList;

	bool 				mCheckComments;
	Breakpoints 		mBreakpoints;
	ErrorMarkers 		mErrorMarkers;
	Vec2 				mCharAdvance;
	Coordinates 		mInteractiveStart, mInteractiveEnd;
	std::string 		mLineBuffer;
	uint64_t 			mStartTime;

	float 				mLastClick;

	Text_Editor();
	~Text_Editor();

	void 				SetLanguageDefinition(const LanguageDefinition& aLanguageDef);
	const LanguageDefinition&
						GetLanguageDefinition() const { return mLanguageDefinition; }

	void 				SetText(const std::string& aText);
	std::string 		GetText() const;

	void 				SetTextLines(const std::vector<std::string>& aLines);
	std::vector<std::string>
						GetTextLines() const;

	std::string 		GetSelectedText() const;
	std::string 		GetCurrentLineText()const;

	int 				GetTotalLines() const { return (int)mLines.size(); }

	void 				SetReadOnly(bool aValue);

	bool 				IsColorizerEnabled() const { return mColorizerEnabled; }
	void 				SetColorizerEnable(bool aValue);

	Coordinates 		GetCursorPosition() const { return GetActualCursorCoordinates(); }
	void 				SetCursorPosition(const Coordinates& aPosition);

	void 				SetTabSize(int aValue);
	inline int 			GetTabSize() const { return mTabSize; }

	void 				InsertText(const std::string& aValue, bool aSelect = false);
	void 				InsertText(const char* aValue, bool aSelect = false);

	void 				MoveUp(int aAmount = 1, bool aSelect = false);
	void 				MoveDown(int aAmount = 1, bool aSelect = false);
	void 				MoveLeft(int aAmount = 1, bool aSelect = false, bool aWordMode = false);
	void 				MoveRight(int aAmount = 1, bool aSelect = false, bool aWordMode = false);
	void 				MoveTop(bool aSelect = false);
	void 				MoveBottom(bool aSelect = false);
	void 				MoveHome(bool aSelect = false);
	void 				MoveEnd(bool aSelect = false);

	void 				SetSelectionStart(const Coordinates& aPosition);
	void 				SetSelectionEnd(const Coordinates& aPosition);
	const Coordinates 	GetSelectionStart()const;
	const Coordinates 	GetSelectionEnd()const;
	void 				SetSelection(const Coordinates& aStart, const Coordinates& aEnd, SelectionMode aMode = SelectionMode::Normal);
	void 				SelectWordUnderCursor();
	void 				SelectAll();
	bool 				HasSelection() const;

	void 				Copy();
	void 				Cut();
	void 				Paste();
	void 				Delete();

	bool 				CanUndo() const;
	bool 				CanRedo() const;
	void 				Undo(int aSteps = 1);
	void 				Redo(int aSteps = 1);

	size_t				Size() const;

	void  				ProcessInputs();
	void  				Colorize(int aFromLine = 0, int aCount = -1);
	void  				ColorizeRange(int aFromLine = 0, int aToLine = 0);
	void  				ColorizeInternal();
	int   				GetPageSize(float window_height) const;
	std::string 		GetText(const Coordinates& aStart, const Coordinates& aEnd) const;
	Coordinates 		GetActualCursorCoordinates() const;
	Coordinates 		SanitizeCoordinates(const Coordinates& aValue) const;
	void 				Advance(Coordinates& aCoordinates) const;
	void 				DeleteRange(const Coordinates& aStart, const Coordinates& aEnd);
	int  				InsertTextAt(Coordinates& aWhere, const char* aValue);
	void 				AddUndo(UndoRecord& aValue);
	Coordinates 		FindWordStart(const Coordinates& aFrom) const;
	Coordinates 		FindWordEnd(const Coordinates& aFrom) const;
	Coordinates 		FindNextWord(const Coordinates& aFrom) const;
	int 				GetCharacterIndex(const Coordinates& aCoordinates) const;
	int 				GetCharacterColumn(int aLine, int aIndex) const;
	int 				GetLineCharacterCount(int aLine) const;
	int 				GetLineMaxColumn(int aLine) const;
	bool 				IsOnWordBoundary(const Coordinates& aAt) const;
	void 				RemoveLine(int aStart, int aEnd);
	void 				RemoveLine(int aIndex);
	Line& 				InsertLine(int aIndex);
	void 				EnterCharacter(unsigned int aChar, bool aShift);
	void 				Backspace();
	void 				DeleteSelection();
	std::string 		GetWordUnderCursor() const;
	std::string 		GetWordAt(const Coordinates& aCoords) const;
};

inline int ImTextCharToUtf8(char* buf, int buf_size, unsigned int c); // "Borrowed" from ImGui source
int        UTF8CharLength(unsigned int c);

} // namespace ndbl