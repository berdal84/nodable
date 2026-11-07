# Headless Text Editor

I started this file from BalazsJako's [ImGuiColorTextEdit](https://github.com/BalazsJako/ImGuiColorTextEdit).

I am not yet done with it, but my plan is to:
- 0) switch from classes + methods to structs + procedures + few operators, no private stuff except rare cases.
- 1) extract any ImGui dependency. I want an Headless Text Editor, that you can plug to any UI.
- 2) undo/redo must be extendable (user should be able to control it externally, since text edits might be only a subset of edits depending on the software written)
- 3) be able to set malloc/realloc/free procedures

