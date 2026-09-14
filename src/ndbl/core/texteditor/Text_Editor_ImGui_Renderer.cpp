#include "Text_Editor.h"
#include <algorithm>
#include <chrono>
#include <string>
#include <regex>
#include <cmath>

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h> // for imGui::GetCurrentWindow()

namespace ndbl
{
	using Coordinates  = Text_Editor::Coordinates;
	using PaletteIndex = Text_Editor::PaletteIndex;

// ImGui implementation
void text_editor_render(Text_Editor& editor, const char* title, const ImVec2& size, bool border, bool ignoreImGuiChild)
{
	editor.mWithinRender = true;
	editor.mTextChanged  = false;
	editor.mCursorPositionChanged = false;

	ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::ColorConvertU32ToFloat4(editor.mPalette[(int)Text_Editor::PaletteIndex::Background]));
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
	if (!ignoreImGuiChild)
		ImGui::BeginChild(title, size, border, ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_AlwaysHorizontalScrollbar | ImGuiWindowFlags_NoMove);

	if (editor.mHandleKeyboardInputs)
	{
		editor.HandleKeyboardInputs();
		ImGui::PushAllowKeyboardFocus(true);
	}

	if (editor.mHandleMouseInputs)
		editor.HandleMouseInputs();

	editor.ColorizeInternal();
	
    // RENDER

    /* Compute mCharAdvance regarding to scaled font size (Ctrl + mouse wheel)*/
	const float fontSize = ImGui::GetFont()->CalcTextSizeA(ImGui::GetFontSize(), FLT_MAX, -1.0f, "#", nullptr, nullptr).x;
	editor.mCharAdvance = ImVec2(fontSize, ImGui::GetTextLineHeightWithSpacing() * editor.mLineSpacing);

	/* Update palette with the current alpha from style */
	for (int i = 0; i < (int)Text_Editor::PaletteIndex::Max; ++i)
	{
		auto color = ImGui::ColorConvertU32ToFloat4(editor.mPaletteBase[i]);
		color.w *= ImGui::GetStyle().Alpha;
		editor.mPalette[i] = ImGui::ColorConvertFloat4ToU32(color);
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
			longest = std::max(editor.mTextStart + editor.TextDistanceToLineStart(Coordinates(lineNo, editor.GetLineMaxColumn(lineNo))), longest);
			auto columnNo = 0;
			Coordinates lineStartCoord(lineNo, 0);
			Coordinates lineEndCoord(lineNo, editor.GetLineMaxColumn(lineNo));

			// Draw selection for the current line
			float sstart = -1.0f;
			float ssend = -1.0f;

			assert(editor.mState.mSelectionStart <= editor.mState.mSelectionEnd);
			if (editor.mState.mSelectionStart <= lineEndCoord)
				sstart = editor.mState.mSelectionStart > lineStartCoord ? editor.TextDistanceToLineStart(editor.mState.mSelectionStart) : 0.0f;
			if (editor.mState.mSelectionEnd > lineStartCoord)
				ssend = editor.TextDistanceToLineStart(editor.mState.mSelectionEnd < lineEndCoord ? editor.mState.mSelectionEnd : lineEndCoord);

			if (editor.mState.mSelectionEnd.mLine > lineNo)
				ssend += editor.mCharAdvance.x;

			if (sstart != -1 && ssend != -1 && sstart < ssend)
			{
				ImVec2 vstart(lineStartScreenPos.x + editor.mTextStart + sstart, lineStartScreenPos.y);
				ImVec2 vend(lineStartScreenPos.x + editor.mTextStart + ssend, lineStartScreenPos.y + editor.mCharAdvance.y);
				drawList->AddRectFilled(vstart, vend, editor.mPalette[(int)PaletteIndex::Selection]);
			}

			// Draw breakpoints
			auto start = ImVec2(lineStartScreenPos.x + scrollX, lineStartScreenPos.y);

			if (editor.mBreakpoints.count(lineNo + 1) != 0)
			{
				auto end = ImVec2(lineStartScreenPos.x + contentSize.x + 2.0f * scrollX, lineStartScreenPos.y + editor.mCharAdvance.y);
				drawList->AddRectFilled(start, end, editor.mPalette[(int)PaletteIndex::Breakpoint]);
			}

			// Draw error markers
			auto errorIt = editor.mErrorMarkers.find(lineNo + 1);
			if (errorIt != editor.mErrorMarkers.end())
			{
				auto end = ImVec2(lineStartScreenPos.x + contentSize.x + 2.0f * scrollX, lineStartScreenPos.y + editor.mCharAdvance.y);
				drawList->AddRectFilled(start, end, editor.mPalette[(int)PaletteIndex::ErrorMarker]);

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
			drawList->AddText(ImVec2(lineStartScreenPos.x + editor.mTextStart - lineNoWidth, lineStartScreenPos.y), editor.mPalette[(int)PaletteIndex::LineNumber], buf);

			if (editor.mState.mCursorPosition.mLine == lineNo)
			{
				auto focused = ImGui::IsWindowFocused();

				// Highlight the current line (where the cursor is)
				if (!editor.HasSelection())
				{
					auto end = ImVec2(start.x + contentSize.x + scrollX, start.y + editor.mCharAdvance.y);
					drawList->AddRectFilled(start, end, editor.mPalette[(int)(focused ? PaletteIndex::CurrentLineFill : PaletteIndex::CurrentLineFillInactive)]);
					drawList->AddRect(start, end, editor.mPalette[(int)PaletteIndex::CurrentLineEdge], 1.0f);
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
						float cx = editor.TextDistanceToLineStart(editor.mState.mCursorPosition);

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
						drawList->AddRectFilled(cstart, cend, editor.mPalette[(int)PaletteIndex::Cursor]);
						if (elapsed > 800)
							editor.mStartTime = timeEnd;
					}
				}
			}

			// Render colorized text
			auto prevColor = line.empty() ? editor.mPalette[(int)PaletteIndex::Default] : editor.GetGlyphColor(line[0]);
			ImVec2 bufferOffset;

			for (int i = 0; i < line.size();)
			{
				auto& glyph = line[i];
				auto color = editor.GetGlyphColor(glyph);

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
			auto id = editor.GetWordAt(editor.ScreenPosToCoordinates(ImGui::GetMousePos()));
			if (!id.empty())
			{
				auto it = editor.mLanguageDefinition.mIdentifiers.find(id);
				if (it != editor.mLanguageDefinition.mIdentifiers.end())
				{
					ImGui::BeginTooltip();
					ImGui::TextUnformatted(it->second.mDeclaration.c_str());
					ImGui::EndTooltip();
				}
				else
				{
					auto pi = editor.mLanguageDefinition.mPreprocIdentifiers.find(id);
					if (pi != editor.mLanguageDefinition.mPreprocIdentifiers.end())
					{
						ImGui::BeginTooltip();
						ImGui::TextUnformatted(pi->second.mDeclaration.c_str());
						ImGui::EndTooltip();
					}
				}
			}
		}
	}


	ImGui::Dummy(ImVec2((longest + 2), editor.mLines.size() * editor.mCharAdvance.y));

	if (editor.mScrollToCursor)
	{
		editor.EnsureCursorVisible();
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

} // namespace ndbl