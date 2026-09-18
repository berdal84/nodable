#include "Text_Editor_ImGui_Renderer.h"
#include <algorithm>
#include <chrono>
#include <string>
#include <regex>
#include <cmath>
#include "ndbl/core/language/Nodlang.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h> // for imGui::GetCurrentWindow()

namespace ndbl
{

using Coordinates  = Text_Editor::Coordinates;
using PaletteIndex = Text_Editor::PaletteIndex;

void 		text_editor_handle_keyboard_inputs(Text_Editor&);
void 		text_editor_handle_mouse_inputs(Text_Editor&);
void 		text_editor_ensure_cursor_visible(Text_Editor&);
float 		text_editor_text_distance_to_line_start(const Text_Editor&, const Coordinates& from);
Coordinates text_editor_screen_pos_to_coordinates(const Text_Editor&, const Vec2& position);
ImColor     text_editor_get_glyph_color(const Text_Editor& editor, const Palette& palette, const Text_Editor::Glyph& glyph);

void text_editor_ensure_cursor_visible(Text_Editor& editor)
{
	if (!editor.mWithinRender)
	{
		editor.mScrollToCursor = true;
		return;
	}

	float scrollX = ImGui::GetScrollX();
	float scrollY = ImGui::GetScrollY();

	auto height = ImGui::GetWindowHeight();
	auto width = ImGui::GetWindowWidth();

	auto top = 1 + (int)ceil(scrollY / editor.mCharAdvance.y);
	auto bottom = (int)ceil((scrollY + height) / editor.mCharAdvance.y);

	auto left = (int)ceil(scrollX / editor.mCharAdvance.x);
	auto right = (int)ceil((scrollX + width) / editor.mCharAdvance.x);

	auto pos = editor.GetActualCursorCoordinates();
	auto len = text_editor_text_distance_to_line_start(editor, pos);

	if (pos.mLine < top)
		ImGui::SetScrollY(std::max(0.0f, (pos.mLine - 1) * editor.mCharAdvance.y));
	if (pos.mLine > bottom - 4)
		ImGui::SetScrollY(std::max(0.0f, (pos.mLine + 4) * editor.mCharAdvance.y - height));
	if (len + editor.mTextStart < left + 4)
		ImGui::SetScrollX(std::max(0.0f, len + editor.mTextStart - 4));
	if (len + editor.mTextStart > right - 4)
		ImGui::SetScrollX(std::max(0.0f, len + editor.mTextStart + 4 - width));
}

void text_editor_handle_keyboard_inputs(Text_Editor& editor)
{
	ImGuiIO& io = ImGui::GetIO();
	auto shift = io.KeyShift;
	auto ctrl = io.ConfigMacOSXBehaviors ? io.KeySuper : io.KeyCtrl;
	auto alt = io.ConfigMacOSXBehaviors ? io.KeyCtrl : io.KeyAlt;

	if (ImGui::IsWindowFocused())
	{
		if (ImGui::IsWindowHovered())
			ImGui::SetMouseCursor(ImGuiMouseCursor_TextInput);
		//ImGui::CaptureKeyboardFromApp(true);

		io.WantCaptureKeyboard = true;
		io.WantTextInput = true;

		if (!editor.mReadOnly && ctrl && !shift && !alt && ImGui::IsKeyPressed(ImGuiKey_Z))
			editor.Undo();
		else if (!editor.mReadOnly && !ctrl && !shift && alt && ImGui::IsKeyPressed(ImGuiKey_Backspace))
			editor.Undo();
		else if (!editor.mReadOnly && ctrl && !shift && !alt && ImGui::IsKeyPressed(ImGuiKey_Y))
			editor.Redo();
		else if (!ctrl && !alt && ImGui::IsKeyPressed(ImGuiKey_UpArrow))
			editor.MoveUp(1, shift);
		else if (!ctrl && !alt && ImGui::IsKeyPressed(ImGuiKey_DownArrow))
			editor.MoveDown(1, shift);
		else if (!alt && ImGui::IsKeyPressed(ImGuiKey_LeftArrow))
			editor.MoveLeft(1, shift, ctrl);
		else if (!alt && ImGui::IsKeyPressed(ImGuiKey_RightArrow))
			editor.MoveRight(1, shift, ctrl);
		else if (!alt && ImGui::IsKeyPressed(ImGuiKey_PageUp))
			editor.MoveUp(editor.GetPageSize( ImGui::GetWindowHeight()) - 4, shift);
		else if (!alt && ImGui::IsKeyPressed(ImGuiKey_PageDown))
			editor.MoveDown(editor.GetPageSize(ImGui::GetWindowHeight()) - 4, shift);
		else if (!alt && ctrl && ImGui::IsKeyPressed(ImGuiKey_Home))
			editor.MoveTop(shift);
		else if (ctrl && !alt && ImGui::IsKeyPressed(ImGuiKey_End))
			editor.MoveBottom(shift);
		else if (!ctrl && !alt && ImGui::IsKeyPressed(ImGuiKey_Home))
			editor.MoveHome(shift);
		else if (!ctrl && !alt && ImGui::IsKeyPressed(ImGuiKey_End))
			editor.MoveEnd(shift);
		else if (!editor.mReadOnly && !ctrl && !shift && !alt && ImGui::IsKeyPressed(ImGuiKey_Delete))
			editor.Delete();
		else if (!editor.mReadOnly && !ctrl && !shift && !alt && ImGui::IsKeyPressed(ImGuiKey_Backspace))
			editor.Backspace();
		else if (!ctrl && !shift && !alt && ImGui::IsKeyPressed(ImGuiKey_Insert))
			editor.mOverwrite ^= true;
		else if (ctrl && !shift && !alt && ImGui::IsKeyPressed(ImGuiKey_Insert))
			editor.Copy();
		else if (ctrl && !shift && !alt && ImGui::IsKeyPressed(ImGuiKey_C))
			editor.Copy();
		else if (!editor.mReadOnly && !ctrl && shift && !alt && ImGui::IsKeyPressed(ImGuiKey_Insert))
			editor.Paste();
		else if (!editor.mReadOnly && ctrl && !shift && !alt && ImGui::IsKeyPressed(ImGuiKey_V))
			editor.Paste();
		else if (ctrl && !shift && !alt && ImGui::IsKeyPressed(ImGuiKey_X))
			editor.Cut();
		else if (!ctrl && shift && !alt && ImGui::IsKeyPressed(ImGuiKey_Delete))
			editor.Cut();
		else if (ctrl && !shift && !alt && ImGui::IsKeyPressed(ImGuiKey_A))
			editor.SelectAll();
		else if (!editor.mReadOnly && !ctrl && !shift && !alt && ImGui::IsKeyPressed(ImGuiKey_Enter))
			editor.EnterCharacter('\n', false);
		else if (!editor.mReadOnly && !ctrl && !alt && ImGui::IsKeyPressed(ImGuiKey_Tab))
			editor.EnterCharacter('\t', shift);

		if (!editor.mReadOnly && !io.InputQueueCharacters.empty())
		{
			for (int i = 0; i < io.InputQueueCharacters.Size; i++)
			{
				auto c = io.InputQueueCharacters[i];
				if (c != 0 && (c == '\n' || c >= 32))
				{
					editor.EnterCharacter(c, shift);
				}
			}
			io.InputQueueCharacters.resize(0);
		}
	}
}

void text_editor_handle_mouse_inputs(Text_Editor& editor)
{
	ImGuiIO& io = ImGui::GetIO();
	auto shift = io.KeyShift;
	auto ctrl = io.ConfigMacOSXBehaviors ? io.KeySuper : io.KeyCtrl;
	auto alt = io.ConfigMacOSXBehaviors ? io.KeyCtrl : io.KeyAlt;

	if (ImGui::IsWindowHovered())
	{
		if (!shift && !alt)
		{
			auto click = ImGui::IsMouseClicked(0);
			auto doubleClick = ImGui::IsMouseDoubleClicked(0);
			auto t = ImGui::GetTime();
			auto tripleClick = click && !doubleClick && (editor.mLastClick != -1.0f && (t - editor.mLastClick) < io.MouseDoubleClickTime);

			/*
			Left mouse button triple click
			*/

			if (tripleClick)
			{
				if (!ctrl)
				{
					editor.mState.mCursorPosition = editor.mInteractiveStart = editor.mInteractiveEnd = text_editor_screen_pos_to_coordinates(editor, ImGui::GetMousePos());
					editor.mSelectionMode = Text_Editor::SelectionMode::Line;
					editor.SetSelection(editor.mInteractiveStart, editor.mInteractiveEnd, editor.mSelectionMode);
				}

				editor.mLastClick = -1.0f;
			}

			/*
			Left mouse button double click
			*/

			else if (doubleClick)
			{
				if (!ctrl)
				{
					editor.mState.mCursorPosition = editor.mInteractiveStart = editor.mInteractiveEnd = text_editor_screen_pos_to_coordinates(editor, ImGui::GetMousePos());
					if (editor.mSelectionMode == Text_Editor::SelectionMode::Line)
						editor.mSelectionMode = Text_Editor::SelectionMode::Normal;
					else
						editor.mSelectionMode = Text_Editor::SelectionMode::Word;
					editor.SetSelection(editor.mInteractiveStart, editor.mInteractiveEnd, editor.mSelectionMode);
				}

				editor.mLastClick = (float)ImGui::GetTime();
			}

			/*
			Left mouse button click
			*/
			else if (click)
			{
				editor.mState.mCursorPosition = editor.mInteractiveStart = editor.mInteractiveEnd = text_editor_screen_pos_to_coordinates(editor, ImGui::GetMousePos());
				if (ctrl)
					editor.mSelectionMode = Text_Editor::SelectionMode::Word;
				else
					editor.mSelectionMode = Text_Editor::SelectionMode::Normal;
				editor.SetSelection(editor.mInteractiveStart, editor.mInteractiveEnd, editor.mSelectionMode);

				editor.mLastClick = (float)ImGui::GetTime();
			}
			// Mouse left button dragging (=> update selection)
			else if (ImGui::IsMouseDragging(0) && ImGui::IsMouseDown(0))
			{
				io.WantCaptureMouse = true;
				editor.mState.mCursorPosition = editor.mInteractiveEnd = text_editor_screen_pos_to_coordinates(editor, ImGui::GetMousePos());
				editor.SetSelection(editor.mInteractiveStart, editor.mInteractiveEnd, editor.mSelectionMode);
			}
		}
	}
}

float text_editor_text_distance_to_line_start(const Text_Editor& editor, const Coordinates& aFrom)
{
	auto& line = editor.mLines[aFrom.mLine];
	float distance = 0.0f;
	float spaceSize = ImGui::GetFont()->CalcTextSizeA(ImGui::GetFontSize(), FLT_MAX, -1.0f, " ", nullptr, nullptr).x;
	int colIndex = editor.GetCharacterIndex(aFrom);
	for (size_t it = 0u; it < line.size() && it < colIndex; )
	{
		if (line[it].mChar == '\t')
		{
			distance = (1.0f + std::floor((1.0f + distance) / (float(editor.mTabSize) * spaceSize))) * (float(editor.mTabSize) * spaceSize);
			++it;
		}
		else
		{
			auto d = UTF8CharLength(line[it].mChar);
			char tempCString[7];
			int i = 0;
			for (; i < 6 && d-- > 0 && it < (int)line.size(); i++, it++)
				tempCString[i] = line[it].mChar;

			tempCString[i] = '\0';
			distance += ImGui::GetFont()->CalcTextSizeA(ImGui::GetFontSize(), FLT_MAX, -1.0f, tempCString, nullptr, nullptr).x;
		}
	}

	return distance;
}

// ImGui implementation
void text_editor_render(Text_Editor& editor, const char* title, const Palette& paletteBase, const ImVec2& size, bool border, bool ignoreImGuiChild)
{
	editor.mWithinRender = true;
	editor.mTextChanged  = false;
	editor.mCursorPositionChanged = false;

	auto palette = paletteBase; // We'll mutate some values

	ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::ColorConvertU32ToFloat4(palette[(int)Text_Editor::PaletteIndex::Background]));
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
	if (!ignoreImGuiChild)
		ImGui::BeginChild(title, size, border, ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_AlwaysHorizontalScrollbar | ImGuiWindowFlags_NoMove);

	if (editor.mHandleKeyboardInputs)
	{
		text_editor_handle_keyboard_inputs(editor);
		ImGui::PushAllowKeyboardFocus(true);
	}

	if (editor.mHandleMouseInputs)
	{
		text_editor_handle_mouse_inputs(editor);
	}

	text_editor_ensure_cursor_visible(editor);

	// Colorize: TODO
	
    // RENDER

    /* Compute mCharAdvance regarding to scaled font size (Ctrl + mouse wheel)*/
	const float fontSize = ImGui::GetFont()->CalcTextSizeA(ImGui::GetFontSize(), FLT_MAX, -1.0f, "#", nullptr, nullptr).x;
	editor.mCharAdvance = ImVec2(fontSize, ImGui::GetTextLineHeightWithSpacing() * editor.mLineSpacing);

	/* Update palette with the current alpha from style */
	for (int i = 0; i < (int)Text_Editor::PaletteIndex::Max; ++i)
	{
		auto color = ImGui::ColorConvertU32ToFloat4(palette[i]);
		color.w *= ImGui::GetStyle().Alpha;
		palette[i] = ImGui::ColorConvertFloat4ToU32(color);
	}

	assert(editor.mLineBuffer.empty());

	auto contentSize = ImGui::GetWindowContentRegionMax();
	auto drawList = ImGui::GetWindowDrawList();
	float longest(editor.mTextStart);

	if (editor.mScrollToTop)
	{
		editor.mScrollToTop = false;
		ImGui::SetScrollY(0.f);
	}

	ImVec2 cursorScreenPos = ImGui::GetCursorScreenPos();
	auto scrollX = ImGui::GetScrollX();
	auto scrollY = ImGui::GetScrollY();

	auto lineNo = (int)floor(scrollY / editor.mCharAdvance.y);
	auto globalLineMax = (int)editor.mLines.size();
	auto lineMax = std::max(0, std::min((int)editor.mLines.size() - 1, lineNo + (int)floor((scrollY + contentSize.y) / editor.mCharAdvance.y)));

	// Deduce mTextStart by evaluating mLines size (global lineMax) plus two spaces as text width
	char buf[16];
	snprintf(buf, 16, " %d ", globalLineMax);
	editor.mTextStart = ImGui::GetFont()->CalcTextSizeA(ImGui::GetFontSize(), FLT_MAX, -1.0f, buf, nullptr, nullptr).x + editor.mLeftMargin;

	if (!editor.mLines.empty())
	{
		float spaceSize = ImGui::GetFont()->CalcTextSizeA(ImGui::GetFontSize(), FLT_MAX, -1.0f, " ", nullptr, nullptr).x;

		while (lineNo <= lineMax)
		{
			ImVec2 lineStartScreenPos = ImVec2(cursorScreenPos.x, cursorScreenPos.y + lineNo * editor.mCharAdvance.y);
			ImVec2 textScreenPos = ImVec2(lineStartScreenPos.x + editor.mTextStart, lineStartScreenPos.y);

			auto& line = editor.mLines[lineNo];
			longest = std::max(editor.mTextStart + text_editor_text_distance_to_line_start(editor, Coordinates(lineNo, editor.GetLineMaxColumn(lineNo))), longest);
			auto columnNo = 0;
			Coordinates lineStartCoord(lineNo, 0);
			Coordinates lineEndCoord(lineNo, editor.GetLineMaxColumn(lineNo));

			// Draw selection for the current line
			float sstart = -1.0f;
			float ssend = -1.0f;

			assert(editor.mState.mSelectionStart <= editor.mState.mSelectionEnd);
			if (editor.mState.mSelectionStart <= lineEndCoord)
				sstart = editor.mState.mSelectionStart > lineStartCoord ? text_editor_text_distance_to_line_start(editor, editor.mState.mSelectionStart) : 0.0f;
			if (editor.mState.mSelectionEnd > lineStartCoord)
				ssend = text_editor_text_distance_to_line_start(editor, editor.mState.mSelectionEnd < lineEndCoord ? editor.mState.mSelectionEnd : lineEndCoord);

			if (editor.mState.mSelectionEnd.mLine > lineNo)
				ssend += editor.mCharAdvance.x;

			if (sstart != -1 && ssend != -1 && sstart < ssend)
			{
				ImVec2 vstart(lineStartScreenPos.x + editor.mTextStart + sstart, lineStartScreenPos.y);
				ImVec2 vend(lineStartScreenPos.x + editor.mTextStart + ssend, lineStartScreenPos.y + editor.mCharAdvance.y);
				drawList->AddRectFilled(vstart, vend, palette[(int)PaletteIndex::Selection]);
			}

			// Draw breakpoints
			auto start = ImVec2(lineStartScreenPos.x + scrollX, lineStartScreenPos.y);

			if (editor.mBreakpoints.count(lineNo + 1) != 0)
			{
				auto end = ImVec2(lineStartScreenPos.x + contentSize.x + 2.0f * scrollX, lineStartScreenPos.y + editor.mCharAdvance.y);
				drawList->AddRectFilled(start, end, palette[(int)PaletteIndex::Breakpoint]);
			}

			// Draw error markers
			auto errorIt = editor.mErrorMarkers.find(lineNo + 1);
			if (errorIt != editor.mErrorMarkers.end())
			{
				auto end = ImVec2(lineStartScreenPos.x + contentSize.x + 2.0f * scrollX, lineStartScreenPos.y + editor.mCharAdvance.y);
				drawList->AddRectFilled(start, end, palette[(int)PaletteIndex::ErrorMarker]);

				if (ImGui::IsMouseHoveringRect(lineStartScreenPos, end))
				{
					ImGui::BeginTooltip();
					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.2f, 0.2f, 1.0f));
					ImGui::Text("Error at line %d:", errorIt->first);
					ImGui::PopStyleColor();
					ImGui::Separator();
					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.2f, 1.0f));
					ImGui::Text("%s", errorIt->second.c_str());
					ImGui::PopStyleColor();
					ImGui::EndTooltip();
				}
			}

			// Draw line number (right aligned)
			snprintf(buf, 16, "%d  ", lineNo + 1);

			auto lineNoWidth = ImGui::GetFont()->CalcTextSizeA(ImGui::GetFontSize(), FLT_MAX, -1.0f, buf, nullptr, nullptr).x;
			drawList->AddText(ImVec2(lineStartScreenPos.x + editor.mTextStart - lineNoWidth, lineStartScreenPos.y), palette[(int)PaletteIndex::LineNumber], buf);

			if (editor.mState.mCursorPosition.mLine == lineNo)
			{
				auto focused = ImGui::IsWindowFocused();

				// Highlight the current line (where the cursor is)
				if (!editor.HasSelection())
				{
					auto end = ImVec2(start.x + contentSize.x + scrollX, start.y + editor.mCharAdvance.y);
					drawList->AddRectFilled(start, end, palette[(int)(focused ? PaletteIndex::CurrentLineFill : PaletteIndex::CurrentLineFillInactive)]);
					drawList->AddRect(start, end, palette[(int)PaletteIndex::CurrentLineEdge], 1.0f);
				}

				// Render the cursor
				if (focused)
				{
					auto timeEnd = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
					auto elapsed = timeEnd - editor.mStartTime;
					if (elapsed > 400)
					{
						float width = 1.0f;
						auto cindex = editor.GetCharacterIndex(editor.mState.mCursorPosition);
						float cx = text_editor_text_distance_to_line_start(editor, editor.mState.mCursorPosition);

						if (editor.mOverwrite && cindex < (int)line.size())
						{
							auto c = line[cindex].mChar;
							if (c == '\t')
							{
								auto x = (1.0f + std::floor((1.0f + cx) / (float(editor.mTabSize) * spaceSize))) * (float(editor.mTabSize) * spaceSize);
								width = x - cx;
							}
							else
							{
								char buf2[2];
								buf2[0] = line[cindex].mChar;
								buf2[1] = '\0';
								width = ImGui::GetFont()->CalcTextSizeA(ImGui::GetFontSize(), FLT_MAX, -1.0f, buf2).x;
							}
						}
						ImVec2 cstart(textScreenPos.x + cx, lineStartScreenPos.y);
						ImVec2 cend(textScreenPos.x + cx + width, lineStartScreenPos.y + editor.mCharAdvance.y);
						drawList->AddRectFilled(cstart, cend, palette[(int)PaletteIndex::Cursor]);
						if (elapsed > 800)
							editor.mStartTime = timeEnd;
					}
				}
			}

			// Render colorized text
			auto prevColor = line.empty() ? palette[(int)PaletteIndex::Default] : text_editor_get_glyph_color(editor, palette, line[0]);
			ImVec2 bufferOffset;

			for (int i = 0; i < line.size();)
			{
				auto& glyph = line[i];
				auto color = text_editor_get_glyph_color(editor, palette, glyph);

				if ((color != prevColor || glyph.mChar == '\t' || glyph.mChar == ' ') && !editor.mLineBuffer.empty())
				{
					const ImVec2 newOffset(textScreenPos.x + bufferOffset.x, textScreenPos.y + bufferOffset.y);
					drawList->AddText(newOffset, prevColor, editor.mLineBuffer.c_str());
					auto textSize = ImGui::GetFont()->CalcTextSizeA(ImGui::GetFontSize(), FLT_MAX, -1.0f, editor.mLineBuffer.c_str(), nullptr, nullptr);
					bufferOffset.x += textSize.x;
					editor.mLineBuffer.clear();
				}
				prevColor = color;

				if (glyph.mChar == '\t')
				{
					auto oldX = bufferOffset.x;
					bufferOffset.x = (1.0f + std::floor((1.0f + bufferOffset.x) / (float(editor.mTabSize) * spaceSize))) * (float(editor.mTabSize) * spaceSize);
					++i;

					if (editor.mShowWhitespaces)
					{
						const auto s = ImGui::GetFontSize();
						const auto x1 = textScreenPos.x + oldX + 1.0f;
						const auto x2 = textScreenPos.x + bufferOffset.x - 1.0f;
						const auto y = textScreenPos.y + bufferOffset.y + s * 0.5f;
						const ImVec2 p1(x1, y);
						const ImVec2 p2(x2, y);
						const ImVec2 p3(x2 - s * 0.2f, y - s * 0.2f);
						const ImVec2 p4(x2 - s * 0.2f, y + s * 0.2f);
						drawList->AddLine(p1, p2, 0x90909090);
						drawList->AddLine(p2, p3, 0x90909090);
						drawList->AddLine(p2, p4, 0x90909090);
					}
				}
				else if (glyph.mChar == ' ')
				{
					if (editor.mShowWhitespaces)
					{
						const auto s = ImGui::GetFontSize();
						const auto x = textScreenPos.x + bufferOffset.x + spaceSize * 0.5f;
						const auto y = textScreenPos.y + bufferOffset.y + s * 0.5f;
						drawList->AddCircleFilled(ImVec2(x, y), 1.5f, 0x80808080, 4);
					}
					bufferOffset.x += spaceSize;
					i++;
				}
				else
				{
					auto l = UTF8CharLength(glyph.mChar);
					while (l-- > 0)
						editor.mLineBuffer.push_back(line[i++].mChar);
				}
				++columnNo;
			}

			if (!editor.mLineBuffer.empty())
			{
				const ImVec2 newOffset(textScreenPos.x + bufferOffset.x, textScreenPos.y + bufferOffset.y);
				drawList->AddText(newOffset, prevColor, editor.mLineBuffer.c_str());
				editor.mLineBuffer.clear();
			}

			++lineNo;
		}

		// Draw a tooltip on known identifiers/preprocessor symbols
		if (ImGui::IsMousePosValid())
		{
			auto id = editor.GetWordAt( text_editor_screen_pos_to_coordinates(editor, ImGui::GetMousePos()) );
			if (!id.empty())
			{
				// TODO
			}
		}
	}


	ImGui::Dummy(ImVec2((longest + 2), editor.mLines.size() * editor.mCharAdvance.y));

	if (editor.mScrollToCursor)
	{
		text_editor_ensure_cursor_visible(editor);
		editor.mScrollToCursor = false;
	}

    // RENDER (end)

	if (editor.mHandleKeyboardInputs)
		ImGui::PopAllowKeyboardFocus();

	if (!ignoreImGuiChild)
		ImGui::EndChild();

	ImGui::PopStyleVar();
	ImGui::PopStyleColor();

	editor.mWithinRender = false;
}

Text_Editor::Coordinates text_editor_screen_pos_to_coordinates(const Text_Editor& editor, const Vec2& aPosition)
{
	ImVec2 origin = ImGui::GetCursorScreenPos();
	ImVec2 local(aPosition.x - origin.x, aPosition.y - origin.y);

	int lineNo = std::max(0, (int)floor(local.y / editor.mCharAdvance.y));

	int columnCoord = 0;

	if (lineNo >= 0 && lineNo < (int)editor.mLines.size())
	{
		auto& line = editor.mLines.at(lineNo);

		int columnIndex = 0;
		float columnX = 0.0f;

		while ((size_t)columnIndex < line.size())
		{
			float columnWidth = 0.0f;

			if (line[columnIndex].mChar == '\t')
			{
				float spaceSize = ImGui::GetFont()->CalcTextSizeA(ImGui::GetFontSize(), FLT_MAX, -1.0f, " ").x;
				float oldX = columnX;
				float newColumnX = (1.0f + std::floor((1.0f + columnX) / (float(editor.mTabSize) * spaceSize))) * (float(editor.mTabSize) * spaceSize);
				columnWidth = newColumnX - oldX;
				if (editor.mTextStart + columnX + columnWidth * 0.5f > local.x)
					break;
				columnX = newColumnX;
				columnCoord = (columnCoord / editor.mTabSize) * editor.mTabSize + editor.mTabSize;
				columnIndex++;
			}
			else
			{
				char buf[7];
				auto d = UTF8CharLength(line[columnIndex].mChar);
				int i = 0;
				while (i < 6 && d-- > 0)
					buf[i++] = line[columnIndex++].mChar;
				buf[i] = '\0';
				columnWidth = ImGui::GetFont()->CalcTextSizeA(ImGui::GetFontSize(), FLT_MAX, -1.0f, buf).x;
				if (editor.mTextStart + columnX + columnWidth * 0.5f > local.x)
					break;
				columnX += columnWidth;
				columnCoord++;
			}
		}
	}

	return editor.SanitizeCoordinates(Coordinates(lineNo, columnCoord));
}

ImColor text_editor_get_glyph_color(const Text_Editor& editor, const Palette& palette, const Text_Editor::Glyph& glyph)
{
	if (!editor.mColorizerEnabled)
		return palette[(int)PaletteIndex::Default];
	if (glyph.mComment)
		return palette[(int)PaletteIndex::Comment];
	if (glyph.mMultiLineComment)
		return palette[(int)PaletteIndex::MultiLineComment];
	auto const color = palette[(int)glyph.mColorIndex];
	if (glyph.mPreprocessor)
	{
		const auto ppcolor = palette[(int)PaletteIndex::Preprocessor];
		const int c0 = ((ppcolor & 0xff) + (color & 0xff)) / 2;
		const int c1 = (((ppcolor >> 8) & 0xff) + ((color >> 8) & 0xff)) / 2;
		const int c2 = (((ppcolor >> 16) & 0xff) + ((color >> 16) & 0xff)) / 2;
		const int c3 = (((ppcolor >> 24) & 0xff) + ((color >> 24) & 0xff)) / 2;
		return ImColor(c0 | (c1 << 8) | (c2 << 16) | (c3 << 24));
	}
	return color;
}


const Palette& text_editor_get_dark_palette()
{
	const static Palette p = { {
			0xff7f7f7f,	// Default
			0xffd69c56,	// Keyword	
			0xff00ff00,	// Number
			0xff7070e0,	// String
			0xff70a0e0, // Char literal
			0xffffffff, // Punctuation
			0xff408080,	// Preprocessor
			0xffaaaaaa, // Identifier
			0xff9bc64d, // Known identifier
			0xffc040a0, // Preproc identifier
			0xff206020, // Comment (single line)
			0xff406020, // Comment (multi line)
			0xff101010, // Background
			0xffe0e0e0, // Cursor
			0x80a06020, // Selection
			0x800020ff, // ErrorMarker
			0x40f08000, // Breakpoint
			0xff707000, // Line number
			0x40000000, // Current line fill
			0x40808080, // Current line fill (inactive)
			0x40a0a0a0, // Current line edge
		} };
	return p;
}

const Palette& text_editor_get_light_palette()
{
	const static Palette p = { {
			0xff7f7f7f,	// None
			0xffff0c06,	// Keyword	
			0xff008000,	// Number
			0xff2020a0,	// String
			0xff304070, // Char literal
			0xff000000, // Punctuation
			0xff406060,	// Preprocessor
			0xff404040, // Identifier
			0xff606010, // Known identifier
			0xffc040a0, // Preproc identifier
			0xff205020, // Comment (single line)
			0xff405020, // Comment (multi line)
			0xffffffff, // Background
			0xff000000, // Cursor
			0x80600000, // Selection
			0xa00010ff, // ErrorMarker
			0x80f08000, // Breakpoint
			0xff505000, // Line number
			0x40000000, // Current line fill
			0x40808080, // Current line fill (inactive)
			0x40000000, // Current line edge
		} };
	return p;
}

const Palette& text_editor_get_retro_blue_palette()
{
	const static Palette p = { {
			0xff00ffff,	// None
			0xffffff00,	// Keyword	
			0xff00ff00,	// Number
			0xff808000,	// String
			0xff808000, // Char literal
			0xffffffff, // Punctuation
			0xff008000,	// Preprocessor
			0xff00ffff, // Identifier
			0xffffffff, // Known identifier
			0xffff00ff, // Preproc identifier
			0xff808080, // Comment (single line)
			0xff404040, // Comment (multi line)
			0xff800000, // Background
			0xff0080ff, // Cursor
			0x80ffff00, // Selection
			0xa00000ff, // ErrorMarker
			0x80ff8000, // Breakpoint
			0xff808000, // Line number
			0x40000000, // Current line fill
			0x40808080, // Current line fill (inactive)
			0x40000000, // Current line edge
		} };
	return p;
}


} // namespace ndbl