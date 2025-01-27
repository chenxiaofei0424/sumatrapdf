/* Copyright 2021 the SumatraPDF project authors (see AUTHORS file).
   License: GPLv3 */

struct DrawInstr;
struct EbookControls;
struct EbookFormattingData;
struct FrameRateWnd;

struct EbookController;
class EbookFormattingThread;
class HtmlFormatter;
struct HtmlFormatterArgs;
struct HtmlPage;

namespace mui {
class Control;
}
using namespace mui;

struct EbookController : Controller {
    EbookController(const Doc& doc, EbookControls* ctrls, ControllerCallback* cb);
    ~EbookController() override;

    [[nodiscard]] const WCHAR* GetFilePath() const override {
        return doc.GetFilePath();
    }
    [[nodiscard]] const WCHAR* GetDefaultFileExt() const override {
        return doc.GetDefaultFileExt();
    }
    [[nodiscard]] int PageCount() const override {
        return GetMaxPageCount();
    }
    WCHAR* GetProperty(DocumentProperty prop) override {
        return doc.GetProperty(prop);
    }

    [[nodiscard]] int CurrentPageNo() const override {
        return currPageNo;
    }
    void GoToPage(int pageNo, bool addNavPoint) override;
    [[nodiscard]] bool CanNavigate(int dir) const override;
    void Navigate(int dir) override;

    void SetDisplayMode(DisplayMode mode, bool keepContinuous = false) override;
    [[nodiscard]] DisplayMode GetDisplayMode() const override {
        return IsDoublePage() ? DisplayMode::Facing : DisplayMode::SinglePage;
    }
    void SetPresentationMode(bool) override {
        /* not supported */
    }
    void SetZoomVirtual(float, Point*) override {
        /* not supported */
    }
    [[nodiscard]] float GetZoomVirtual(__unused bool absolute = false) const override {
        return 100;
    }
    [[nodiscard]] float GetNextZoomStep(float) const override {
        return 100;
    }
    void SetViewPortSize(Size size) override;

    TocTree* GetToc() override;
    void ScrollTo(int pageNo, RectF rect, float zoom) override;
    bool HandleLink(IPageDestination*, ILinkHandler*) override;

    IPageDestination* GetNamedDest(const WCHAR* name) override;

    void GetDisplayState(FileState* ds) override;
    void CreateThumbnail(Size size, const onBitmapRenderedCb&) override;

    bool GoToNextPage() override;
    bool GoToPrevPage(bool toBottom = false) override;

    EbookController* AsEbook() override {
        return this;
    }

    // the following is specific to EbookController

    [[nodiscard]] DocType GetDocType() const {
        return doc.Type();
    }
    LRESULT HandleMessage(UINT msg, WPARAM wp, LPARAM lp, bool& wasHandled) const;
    void EnableMessageHandling(bool enable) {
        handleMsgs = enable;
    }
    void UpdateDocumentColors() const;
    void RequestRepaint() const;
    void HandlePagesFromEbookLayout(EbookFormattingData* ft);
    void TriggerLayout();
    void StartLayouting(int startReparseIdxArg = -1, DisplayMode displayMode = DisplayMode::Automatic);
    int ResolvePageAnchor(const WCHAR* id);
    void CopyNavHistory(EbookController& orig);
    [[nodiscard]] int CurrentTocPageNo() const;

    // call StartLayouting before using this EbookController
    static EbookController* Create(const Doc& doc, HWND hwnd, ControllerCallback* cb, FrameRateWnd*);

    static void DeleteEbookFormattingData(EbookFormattingData* data);

    EbookControls* ctrls = nullptr;

    TocTree* tocTree = nullptr;
    Doc doc;

    // TODO: this should be recycled along with pages so that its
    // memory use doesn't grow without bounds
    PoolAllocator textAllocator;

    Vec<HtmlPage*>* pages = nullptr;

    // pages being sent from background formatting thread
    Vec<HtmlPage*>* incomingPages = nullptr;

    // currPageNo is in range 1..$numberOfPages.
    int currPageNo = 0;
    // reparseIdx of the current page (the first one if we're showing 2)
    int currPageReparseIdx = 0;

    // size of the page for which pages were generated
    Size pageSize;

    EbookFormattingThread* formattingThread = nullptr;
    int formattingThreadNo = -1;

    // whether HandleMessage passes messages on to ctrls->mainWnd
    bool handleMsgs = false;

    // parallel lists mapping anchor IDs to reparseIdxs
    WStrVec* pageAnchorIds = nullptr;
    Vec<int>* pageAnchorIdxs = nullptr;

    Vec<int> navHistory;
    size_t navHistoryIdx = 0;

    Vec<HtmlPage*>* GetPages() const;
    void UpdateStatus() const;
    [[nodiscard]] bool FormattingInProgress() const {
        return formattingThread != nullptr;
    }
    void StopFormattingThread();
    void CloseCurrentDocument();
    [[nodiscard]] int GetMaxPageCount() const;
    [[nodiscard]] bool IsDoublePage() const;
    void ExtractPageAnchors();
    void AddNavPoint();
    void OnClickedLink(int pageNo, DrawInstr* link);

    // event handlers
    void ClickedNext(Control* c, int x, int y);
    void ClickedPrev(Control* c, int x, int y);
    void ClickedProgress(Control* c, int x, int y);
    void SizeChangedPage(Control* c, int dx, int dy);
    void ClickedPage1(Control* c, int x, int y);
    void ClickedPage2(Control* c, int x, int y);
};

HtmlFormatterArgs* CreateFormatterArgsDoc(const Doc& doc, int dx, int dy, Allocator* textAllocator = nullptr);
