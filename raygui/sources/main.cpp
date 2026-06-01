#include "raylib.h"

#include <vector>
#include <string>
#include <fstream>
#include <algorithm>
#include <sstream>
#include <cmath>

const int FONT_SIZE     = 20;
const int LINE_HEIGHT   = 24;
const int TEXT_ORIGIN_X = 70;
const int TEXT_ORIGIN_Y = 50;

std::vector<std::string> lines(1, "");

int cursorLine = 0;
int cursorCol  = 0;

// ---------------------------------------------------------------------------
// Selection
// ---------------------------------------------------------------------------
bool selActive    = false;
int  selStartLine = 0;
int  selStartCol  = 0;
int  selEndLine   = 0;
int  selEndCol    = 0;

void GetSelectionOrdered(int& sl, int& sc, int& el, int& ec)
{
    if (selStartLine < selEndLine ||
        (selStartLine == selEndLine && selStartCol <= selEndCol))
    {
        sl = selStartLine; sc = selStartCol;
        el = selEndLine;   ec = selEndCol;
    }
    else
    {
        sl = selEndLine;   sc = selEndCol;
        el = selStartLine; ec = selStartCol;
    }
}

std::string GetSelectedText()
{
    if (!selActive) return "";
    int sl, sc, el, ec;
    GetSelectionOrdered(sl, sc, el, ec);
    std::string result;
    if (sl == el)
    {
        result = lines[sl].substr(sc, ec - sc);
    }
    else
    {
        result = lines[sl].substr(sc);
        for (int i = sl + 1; i < el; i++) result += '\n' + lines[i];
        result += '\n' + lines[el].substr(0, ec);
    }
    return result;
}

void DeleteSelection()
{
    if (!selActive) return;
    int sl, sc, el, ec;
    GetSelectionOrdered(sl, sc, el, ec);
    if (sl == el)
    {
        lines[sl].erase(sc, ec - sc);
    }
    else
    {
        std::string merged = lines[sl].substr(0, sc) + lines[el].substr(ec);
        lines.erase(lines.begin() + sl, lines.begin() + el + 1);
        lines.insert(lines.begin() + sl, merged);
    }
    cursorLine = sl;
    cursorCol  = sc;
    selActive  = false;
}

// Anchor = the fixed end; cursor is the moving end.
// Call this every time the cursor moves with Shift held.
void UpdateSelectionTocursor()
{
    selEndLine = cursorLine;
    selEndCol  = cursorCol;
    selActive  = (selEndLine != selStartLine || selEndCol != selStartCol);
}

// Start a brand-new Shift-selection from the current cursor position.
void BeginShiftSelection()
{
    selStartLine = cursorLine;
    selStartCol  = cursorCol;
}

// ---------------------------------------------------------------------------
// Hit-test
// ---------------------------------------------------------------------------
void PixelToLineCol(int px, int py, int& outLine, int& outCol)
{
    outLine = (py - TEXT_ORIGIN_Y) / LINE_HEIGHT;
    outLine = std::max(0, std::min(outLine, (int)lines.size() - 1));
    int relX = px - TEXT_ORIGIN_X;
    const std::string& ln = lines[outLine];
    outCol = (int)ln.size();
    for (int c = 0; c < (int)ln.size(); c++)
    {
        int w = MeasureText(ln.substr(0, c + 1).c_str(), FONT_SIZE);
        if (relX < w)
        {
            int wPrev = MeasureText(ln.substr(0, c).c_str(), FONT_SIZE);
            outCol = (relX - wPrev < w - relX) ? c : c + 1;
            break;
        }
    }
    outCol = std::max(0, std::min(outCol, (int)ln.size()));
}

// ---------------------------------------------------------------------------
// Accelerating backspace
// ---------------------------------------------------------------------------
float backspaceHeldTime    = 0.0f;
float backspaceRepeatTimer = 0.0f;
const float BACKSPACE_INITIAL_DELAY = 0.4f;
const float BACKSPACE_REPEAT_SLOW   = 0.08f;
const float BACKSPACE_REPEAT_FAST   = 0.016f;
const float BACKSPACE_RAMP_DURATION = 2.0f;

void DoBackspace()
{
    if (selActive) { DeleteSelection(); return; }
    if (cursorCol > 0)
    {
        lines[cursorLine].erase(cursorCol - 1, 1);
        cursorCol--;
    }
    else if (cursorLine > 0)
    {
        int prevLen = (int)lines[cursorLine - 1].size();
        lines[cursorLine - 1] += lines[cursorLine];
        lines.erase(lines.begin() + cursorLine);
        cursorLine--;
        cursorCol = prevLen;
    }
}

// ---------------------------------------------------------------------------
// Clipboard
// ---------------------------------------------------------------------------
void DoCopy()  { if (selActive) SetClipboardText(GetSelectedText().c_str()); }
void DoCut()   { DoCopy(); DeleteSelection(); }
void DoPaste()
{
    if (selActive) DeleteSelection();
    const char* cb = GetClipboardText();
    if (!cb) return;
    std::string cbStr(cb);
    std::istringstream ss{cbStr};
    std::string seg;
    bool first = true;
    while (std::getline(ss, seg))
    {
        if (!first)
        {
            std::string rem = lines[cursorLine].substr(cursorCol);
            lines[cursorLine] = lines[cursorLine].substr(0, cursorCol);
            lines.insert(lines.begin() + cursorLine + 1, rem);
            cursorLine++; cursorCol = 0;
        }
        lines[cursorLine].insert(cursorCol, seg);
        cursorCol += (int)seg.size();
        first = false;
    }
}

// ---------------------------------------------------------------------------
// File I/O
// ---------------------------------------------------------------------------
int GetTotalCharCount()
{
    int t = 0;
    for (const auto& l : lines) t += (int)l.size();
    return t;
}
void SaveFile(const char* fn)
{
    std::ofstream f(fn);
    for (size_t i = 0; i < lines.size(); i++) { f << lines[i]; if (i+1 < lines.size()) f << '\n'; }
}
void LoadFile(const char* fn)
{
    std::ifstream f(fn); if (!f.is_open()) return;
    lines.clear();
    std::string l;
    while (std::getline(f, l)) lines.push_back(l);
    if (lines.empty()) lines.push_back("");
    cursorLine = 0; cursorCol = 0; selActive = false;
}

// ---------------------------------------------------------------------------
// Main
// ---------------------------------------------------------------------------
int main()
{
    InitWindow(1200, 800, "Raylib Text Editor");
    SetTargetFPS(60);

    const char* currentFile = "document.txt";
    bool mouseWasDragging  = false;
    // Track whether the previous Shift+Arrow frame had Shift down,
    // so we know when to anchor a new selection.
    bool shiftWasDown = false;

    while (!WindowShouldClose())
    {
        float dt = GetFrameTime();

        // ----------------------------------------------------------------
        // Mouse
        // ----------------------------------------------------------------
        Vector2 mouse = GetMousePosition();

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            int cl, cc;
            PixelToLineCol((int)mouse.x, (int)mouse.y, cl, cc);

            if (IsKeyDown(KEY_LEFT_SHIFT) && selActive)
            {
                // Shift+click: extend existing selection
                selEndLine = cl; selEndCol = cc;
                selActive  = (selEndLine != selStartLine || selEndCol != selStartCol);
            }
            else
            {
                // Plain click: move cursor, new anchor
                selStartLine = cl; selStartCol = cc;
                selEndLine   = cl; selEndCol   = cc;
                selActive    = false;
            }
            cursorLine = cl; cursorCol = cc;
            mouseWasDragging = false;
        }

        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON))
        {
            int dl, dc;
            PixelToLineCol((int)mouse.x, (int)mouse.y, dl, dc);
            if (dl != selStartLine || dc != selStartCol)
            {
                selEndLine = dl; selEndCol = dc;
                selActive  = (selEndLine != selStartLine || selEndCol != selStartCol);
                cursorLine = dl; cursorCol = dc;
                mouseWasDragging = true;
            }
        }

        if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
            if (!mouseWasDragging) selActive = false;

        // ----------------------------------------------------------------
        // Ctrl shortcuts
        // ----------------------------------------------------------------
        if (IsKeyDown(KEY_LEFT_CONTROL))
        {
            if (IsKeyPressed(KEY_S)) SaveFile(currentFile);
            if (IsKeyPressed(KEY_O)) LoadFile(currentFile);
            if (IsKeyPressed(KEY_C)) DoCopy();
            if (IsKeyPressed(KEY_X)) DoCut();
            if (IsKeyPressed(KEY_V)) DoPaste();
            if (IsKeyPressed(KEY_A))
            {
                selStartLine = 0; selStartCol = 0;
                selEndLine = (int)lines.size()-1; selEndCol = (int)lines.back().size();
                selActive = true;
                cursorLine = selEndLine; cursorCol = selEndCol;
            }
        }
        else
        {
            // ----------------------------------------------------------------
            // Shift+Arrow: extend / start selection
            // ----------------------------------------------------------------
            bool shiftDown = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);

            // If Shift was just pressed (not held last frame), anchor NOW at
            // the current cursor position — before any movement happens.
            if (shiftDown && !shiftWasDown)
                BeginShiftSelection();

            bool movedLeft  = IsKeyPressed(KEY_LEFT);
            bool movedRight = IsKeyPressed(KEY_RIGHT);
            bool movedUp    = IsKeyPressed(KEY_UP);
            bool movedDown  = IsKeyPressed(KEY_DOWN);
            bool anyArrow   = movedLeft || movedRight || movedUp || movedDown;

            if (anyArrow)
            {
                if (shiftDown)
                {
                    // Move cursor
                    if (movedLeft)
                    {
                        if (cursorCol > 0) cursorCol--;
                        else if (cursorLine > 0) { cursorLine--; cursorCol = (int)lines[cursorLine].size(); }
                    }
                    if (movedRight)
                    {
                        if (cursorCol < (int)lines[cursorLine].size()) cursorCol++;
                        else if (cursorLine+1 < (int)lines.size()) { cursorLine++; cursorCol = 0; }
                    }
                    if (movedUp && cursorLine > 0)
                    {
                        cursorLine--;
                        cursorCol = std::min(cursorCol, (int)lines[cursorLine].size());
                    }
                    if (movedDown && cursorLine+1 < (int)lines.size())
                    {
                        cursorLine++;
                        cursorCol = std::min(cursorCol, (int)lines[cursorLine].size());
                    }

                    UpdateSelectionTocursor();
                }
                else
                {
                    // Plain arrow: collapse selection
                    if (selActive)
                    {
                        // Jump to appropriate end of selection
                        int sl, sc, el, ec;
                        GetSelectionOrdered(sl, sc, el, ec);
                        if (movedLeft)  { cursorLine = sl; cursorCol = sc; }
                        if (movedRight) { cursorLine = el; cursorCol = ec; }
                        // Up/Down just move normally after clearing
                        if (movedUp)
                        {
                            if (cursorLine > 0) { cursorLine--; cursorCol = std::min(cursorCol, (int)lines[cursorLine].size()); }
                        }
                        if (movedDown)
                        {
                            if (cursorLine+1 < (int)lines.size()) { cursorLine++; cursorCol = std::min(cursorCol, (int)lines[cursorLine].size()); }
                        }
                    }
                    else
                    {
                        if (movedLeft)
                        {
                            if (cursorCol > 0) cursorCol--;
                            else if (cursorLine > 0) { cursorLine--; cursorCol = (int)lines[cursorLine].size(); }
                        }
                        if (movedRight)
                        {
                            if (cursorCol < (int)lines[cursorLine].size()) cursorCol++;
                            else if (cursorLine+1 < (int)lines.size()) { cursorLine++; cursorCol = 0; }
                        }
                        if (movedUp && cursorLine > 0)
                        {
                            cursorLine--;
                            cursorCol = std::min(cursorCol, (int)lines[cursorLine].size());
                        }
                        if (movedDown && cursorLine+1 < (int)lines.size())
                        {
                            cursorLine++;
                            cursorCol = std::min(cursorCol, (int)lines[cursorLine].size());
                        }
                    }
                    selActive = false;
                }
            }

            shiftWasDown = shiftDown;

            // ----------------------------------------------------------------
            // Text input
            // ----------------------------------------------------------------
            int key = GetCharPressed();
            while (key > 0)
            {
                if (key >= 32 && key <= 126)
                {
                    if (selActive) DeleteSelection();
                    lines[cursorLine].insert(cursorCol, 1, (char)key);
                    cursorCol++;
                }
                key = GetCharPressed();
            }

            // Enter
            if (IsKeyPressed(KEY_ENTER))
            {
                if (selActive) DeleteSelection();
                std::string rem = lines[cursorLine].substr(cursorCol);
                lines[cursorLine] = lines[cursorLine].substr(0, cursorCol);
                lines.insert(lines.begin() + cursorLine + 1, rem);
                cursorLine++; cursorCol = 0;
            }

            // Backspace (accelerating)
            if (IsKeyDown(KEY_BACKSPACE))
            {
                if (IsKeyPressed(KEY_BACKSPACE))
                {
                    DoBackspace();
                    backspaceHeldTime = backspaceRepeatTimer = 0.0f;
                }
                else
                {
                    backspaceHeldTime += dt;
                    if (backspaceHeldTime >= BACKSPACE_INITIAL_DELAY)
                    {
                        float rampT = std::min(1.0f, (backspaceHeldTime - BACKSPACE_INITIAL_DELAY) / BACKSPACE_RAMP_DURATION);
                        float interval = BACKSPACE_REPEAT_SLOW + (BACKSPACE_REPEAT_FAST - BACKSPACE_REPEAT_SLOW) * rampT;
                        backspaceRepeatTimer += dt;
                        while (backspaceRepeatTimer >= interval) { DoBackspace(); backspaceRepeatTimer -= interval; }
                    }
                }
            }
            else { backspaceHeldTime = backspaceRepeatTimer = 0.0f; }
        }

        // ----------------------------------------------------------------
        // Draw
        // ----------------------------------------------------------------
        BeginDrawing();
        ClearBackground(WHITE);

        // Current-line highlight (subtle amber strip)
        if (!selActive)
        {
            DrawRectangle(
                    0,
                    TEXT_ORIGIN_Y + cursorLine * LINE_HEIGHT,
                    GetScreenWidth(),
                    LINE_HEIGHT,
                    (Color){255, 240, 180, 80}
            );
        }

        // Selection highlight
        if (selActive)
        {
            int sl, sc, el, ec;
            GetSelectionOrdered(sl, sc, el, ec);
            for (int i = sl; i <= el; i++)
            {
                const std::string& ln = lines[i];
                int startC = (i == sl) ? sc : 0;
                int endC   = (i == el) ? ec : (int)ln.size();
                int x1 = TEXT_ORIGIN_X + MeasureText(ln.substr(0, startC).c_str(), FONT_SIZE);
                int x2 = TEXT_ORIGIN_X + MeasureText(ln.substr(0, endC).c_str(), FONT_SIZE);
                if (i < el) x2 = TEXT_ORIGIN_X + MeasureText(ln.c_str(), FONT_SIZE) + 8;
                DrawRectangle(x1, TEXT_ORIGIN_Y + i * LINE_HEIGHT, x2 - x1, LINE_HEIGHT, (Color){70, 130, 240, 140});
            }
        }

        // Toolbar
        DrawRectangle(0, 0, GetScreenWidth(), 40, LIGHTGRAY);
        DrawText("Ctrl+S Save | Ctrl+O Open | Ctrl+C Copy | Ctrl+X Cut | Ctrl+V Paste | Ctrl+A All | Shift+Arrows Select",
                 10, 10, 14, GRAY);

        // Char count
        const char* ccText = TextFormat("chars: %d", GetTotalCharCount());
        DrawText(ccText, GetScreenWidth() - MeasureText(ccText, 20) - 10, 10, 20, DARKGRAY);

        // Line numbers gutter
        DrawRectangle(0, 40, 60, GetScreenHeight(), (Color){230, 230, 230, 255});

        // Lines
        for (int i = 0; i < (int)lines.size(); i++)
        {
            Color numColor = (i == cursorLine) ? (Color){60, 60, 200, 255} : GRAY;
            DrawText(TextFormat("%d", i + 1), 5, TEXT_ORIGIN_Y + i * LINE_HEIGHT, FONT_SIZE, numColor);
            DrawText(lines[i].c_str(), TEXT_ORIGIN_X, TEXT_ORIGIN_Y + i * LINE_HEIGHT, FONT_SIZE, (Color){30, 30, 30, 255});
        }

        // Cursor — always drawn; blinks when no selection, solid when selecting
        {
            double t = GetTime();
            bool cursorVisible = selActive ? true : (fmod(t, 1.0) < 0.65);

            if (cursorVisible)
            {
                std::string before = lines[cursorLine].substr(0, cursorCol);
                int cx = TEXT_ORIGIN_X + MeasureText(before.c_str(), FONT_SIZE);
                int cy = TEXT_ORIGIN_Y + cursorLine * LINE_HEIGHT;

                // Wide outer glow
                DrawRectangle(cx - 3, cy - 1, 10, LINE_HEIGHT + 2, (Color){255, 80, 0, 40});
                // Mid glow
                DrawRectangle(cx - 1, cy, 6, LINE_HEIGHT, (Color){255, 120, 20, 90});
                // Main bar — 4 px vivid orange-red, pops against both white and blue selection
                DrawRectangle(cx, cy, 4, LINE_HEIGHT, (Color){255, 60, 0, 255});
                // Bright white centre line for crispness
                DrawRectangle(cx + 1, cy + 2, 2, LINE_HEIGHT - 4, (Color){255, 255, 255, 180});
            }
        }

        EndDrawing();
    }

    CloseWindow();
}