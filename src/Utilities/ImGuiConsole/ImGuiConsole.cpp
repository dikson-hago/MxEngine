#include "ImGuiConsole.h"
#include <cstdlib>
#include <iostream>
#define CHAISCRIPT_NO_THREADS
#include <chaiscript/chaiscript.hpp>

// Portable helpers
static int   Stricmp(const char* str1, const char* str2) { int d; while ((d = toupper(*str2) - toupper(*str1)) == 0 && *str1) { str1++; str2++; } return d; }
static int   Strnicmp(const char* str1, const char* str2, int n) { int d = 0; while (n > 0 && (d = toupper(*str2) - toupper(*str1)) == 0 && *str1) { str1++; str2++; n--; } return d; }
static char* Strdup(const char* str) { size_t len = strlen(str) + 1; void* buf = malloc(len); IM_ASSERT(buf); return (char*)memcpy(buf, (const void*)str, len); }
static void  Strtrim(char* str) { char* str_end = str + strlen(str); while (str_end > str&& str_end[-1] == ' ') str_end--; *str_end = 0; }

namespace MomoEngine
{
	// In C++11 you are better off using lambdas for this sort of forwarding callbacks
	static int TextEditCallbackStub(ImGuiInputTextCallbackData* data)
	{
		ImGuiConsole* console = (ImGuiConsole*)data->UserData;
		return console->TextEditCallback(data);
	}

	ImGuiConsole::ImGuiConsole()
	{
		size = { 450, 500 };
		ClearLog();
		memset(InputBuf, 0, sizeof(InputBuf));
		HistoryPos = -1;
		AutoScroll = true;
		ScrollToBottom = false;
		PrintLog("Welcome to MomoEngine developer console!");
	}

	ImGuiConsole::~ImGuiConsole()
	{
		ClearLog();
		for (int i = 0; i < History.Size; i++)
			free(History[i]);
	}

	void ImGuiConsole::SetChaiScriptObject(chaiscript::ChaiScript* chai)
	{
		this->chai = chai;

		this->chai->add(chaiscript::fun([](ImGuiConsole& console, const std::string& message)
			{
				console.PrintLog("%s", message.c_str());
			}), "print");

		this->chai->add(chaiscript::fun(&ImGuiConsole::ClearLog), "clear");
		this->chai->add(chaiscript::fun(&ImGuiConsole::PrintHistory), "history");

		this->chai->add_global(chaiscript::var(std::ref(*this)), "console");
		this->chai->eval(R"(global print = fun[print](x) { console.print("${ x }"); }; )");

		PrintLog("This console is powered by ChaiScript: http://chaiscript.com");
	}

	void ImGuiConsole::ClearLog()
	{
		for (int i = 0; i < Items.Size; i++)
			free(Items[i]);
		Items.clear();
	}

	void ImGuiConsole::PrintLog(const char* fmt, ...)
	{
		// FIXME-OPT
		char buf[1024];
		va_list args;
		va_start(args, fmt);
		vsnprintf(buf, IM_ARRAYSIZE(buf), fmt, args);
		buf[IM_ARRAYSIZE(buf) - 1] = 0;
		va_end(args);
		Items.push_back(Strdup(buf));
	}

	void ImGuiConsole::Draw(const char* title)
	{
		ImGui::SetNextWindowSize(this->size);
		ImGui::Begin(title);

		if (ImGui::SmallButton("Clear")) { ClearLog(); } ImGui::SameLine();
		bool copy_to_clipboard = ImGui::SmallButton("Copy"); ImGui::SameLine();

		// Options menu
		if (ImGui::BeginPopup("Options"))
		{
			ImGui::Checkbox("Auto-scroll", &AutoScroll);
			ImGui::EndPopup();
		}
		// Options, Filter
		if (ImGui::SmallButton("Options"))
			ImGui::OpenPopup("Options");

		Filter.Draw("Filter", 180);
		ImGui::Separator();

		const float footer_height_to_reserve = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing(); // 1 separator, 1 input text
		ImGui::BeginChild("ScrollingRegion", ImVec2(0, -footer_height_to_reserve), false, ImGuiWindowFlags_HorizontalScrollbar); // Leave room for 1 separator + 1 InputText
		if (ImGui::BeginPopupContextWindow())
		{
			if (ImGui::Selectable("Clear")) ClearLog();
			ImGui::EndPopup();
		}

		// Display every line as a separate entry so we can change their color or add custom widgets. If you only want raw text you can use ImGui::TextUnformatted(log.begin(), log.end());
		// NB- if you have thousands of entries this approach may be too inefficient and may require user-side clipping to only process visible items.
		// You can seek and display only the lines that are visible using the ImGuiListClipper helper, if your elements are evenly spaced and you have cheap random access to the elements.
		// To use the clipper we could replace the 'for (int i = 0; i < Items.Size; i++)' loop with:
		//     ImGuiListClipper clipper(Items.Size);
		//     while (clipper.Step())
		//         for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
		// However, note that you can not use this code as is if a filter is active because it breaks the 'cheap random-access' property. We would need random-access on the post-filtered list.
		// A typical application wanting coarse clipping and filtering may want to pre-compute an array of indices that passed the filtering test, recomputing this array when user changes the filter,
		// and appending newly elements as they are inserted. This is left as a task to the user until we can manage to improve this example code!
		// If your items are of variable size you may want to implement code similar to what ImGuiListClipper does. Or split your data into fixed height items to allow random-seeking into your list.
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 1)); // Tighten spacing
		if (copy_to_clipboard)
		ImGui::LogToClipboard();
		for (int i = 0; i < Items.Size; i++)
		{
			const char* item = Items[i];
			if (!Filter.PassFilter(item))
				continue;

			// Normally you would store more information in your item (e.g. make Items[] an array of structure, store color/type etc.)
			bool pop_color = false;
			if (strstr(item, "[error]:")) { ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.4f, 0.4f, 1.0f)); pop_color = true; }
			else if (strncmp(item, ">> ", 3) == 0) { ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.8f, 0.6f, 1.0f)); pop_color = true; }
			ImGui::TextUnformatted(item);
			if (pop_color)
				ImGui::PopStyleColor();
		}
		if (copy_to_clipboard)
		ImGui::LogFinish();

		if (ScrollToBottom || (AutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY()))
		ImGui::SetScrollHereY(1.0f);
		ScrollToBottom = false;

		ImGui::PopStyleVar();
		ImGui::EndChild();
		ImGui::Separator();

		// Command-line
		bool reclaim_focus = false;
		if (ImGui::InputText("Input", InputBuf, IM_ARRAYSIZE(InputBuf), ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackCompletion | ImGuiInputTextFlags_CallbackHistory, &TextEditCallbackStub, (void*)this))
		{
			char* s = InputBuf;
			Strtrim(s);
			if (s[0])
				ExecCommand(s);
			strcpy_s(s, 1, "");
			reclaim_focus = true;
		}

		// Auto-focus on window apparition
		ImGui::SetItemDefaultFocus();
		if (reclaim_focus)
		ImGui::SetKeyboardFocusHere(-1); // Auto focus previous widget

		ImGui::End();
	}

	ImVec2 ImGuiConsole::GetSize() const
	{
		return this->size;
	}

	void ImGuiConsole::SetSize(ImVec2 size)
	{
		this->size = size;
	}

	void ImGuiConsole::ExecCommand(const char* command_line)
	{
		PrintLog(">> %s\n", command_line);

		// Insert into history. First find match and delete it so it can be pushed to the back. This isn't trying to be smart or optimal.
		HistoryPos = -1;
		for (int i = History.Size - 1; i >= 0; i--)
			if (Stricmp(History[i], command_line) == 0)
			{
				free(History[i]);
				History.erase(History.begin() + i);
				break;
			}
		History.push_back(Strdup(command_line));

		if (this->chai == nullptr) return; // no chai -> no command

		try
		{
			std::string script = command_line;
			chai->eval(script);
		}
		catch (std::exception e)
		{
			std::string error = e.what();
			size_t idx = error.find("Error:");
			if(idx != error.npos)
			{
				error.erase(idx, idx + 6);
				error = "[error]: " + error;
			}
			PrintLog("%s", error.c_str());
		}

		// On commad input, we scroll to bottom even if AutoScroll==false
		ScrollToBottom = true;
	}

	void ImGuiConsole::PrintHistory()
	{
		int first = History.Size - 10;
		for (int i = first > 0 ? first : 0; i < History.Size; i++)
			PrintLog("%3d: %s\n", i, History[i]);
	}

	int ImGuiConsole::TextEditCallback(ImGuiInputTextCallbackData* data)
	{
		//AddLog("cursor: %d, selection: %d-%d", data->CursorPos, data->SelectionStart, data->SelectionEnd);
		switch (data->EventFlag)
		{
		case ImGuiInputTextFlags_CallbackHistory:
		{
			// Example of HISTORY
			const int prev_history_pos = HistoryPos;
			if (data->EventKey == ImGuiKey_UpArrow)
			{
				if (HistoryPos == -1)
					HistoryPos = History.Size - 1;
				else if (HistoryPos > 0)
					HistoryPos--;
			}
			else if (data->EventKey == ImGuiKey_DownArrow)
			{
				if (HistoryPos != -1)
					if (++HistoryPos >= History.Size)
						HistoryPos = -1;
			}

			// A better implementation would preserve the data on the current input line along with cursor position.
			if (prev_history_pos != HistoryPos)
			{
				const char* history_str = (HistoryPos >= 0) ? History[HistoryPos] : "";
				data->DeleteChars(0, data->BufTextLen);
				data->InsertChars(0, history_str);
			}
		}
		}
		return 0;
	}
}