include_directories(${complx-tools_SOURCE_DIR}/lc3test)
include_directories(${complx-tools_SOURCE_DIR}/liblc3)
include_directories(${complx-tools_SOURCE_DIR}/logging)

set(SRC_COMPLX
    AddressDialog.cpp
    AddressDialogDecl.cpp
    BreakpointDialog.cpp
    BreakpointDialogDecl.cpp
    CallStackDialog.cpp
    CallStackDialogDecl.cpp
    CallSubroutineDialog.cpp
    CallSubroutineDialogDecl.cpp
    ComplxApp.cpp
    ComplxFrame.cpp
    ComplxFrameDecl.cpp
    DebugInfoDialog.cpp
    DebugInfoDialogDecl.cpp
    DebugInfoList.cpp
    GridCellBinaryRenderer.cpp
    GridCellInfoRenderer.cpp
    LC3Console.cpp
    LC3ConsoleDecl.cpp
    LC3RunThread.cpp
    MemoryGrid.cpp
    MemoryView.cpp
    MemoryViewContainer.cpp
    MemoryViewFrame.cpp
    MemoryViewFrameDecl.cpp
    RunForDialog.cpp
    RunForDialogDecl.cpp
    RunTestDialog.cpp
    RunTestDialogDecl.cpp
    WatchpointDialog.cpp
    WatchpointDialogDecl.cpp
    icon32.xpm

    ../lc3test/XmlTestParser.cpp
)


add_executable(
    complx
    ${SRC_COMPLX}
)

target_link_libraries(
    complx
    lc3
    ${wxWidgets_LIBRARIES}
)

install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/complx DESTINATION ${CMAKE_INSTALL_BINDIR})

install(FILES ../doc/complx-tools.pdf DESTINATION share/doc/complx-tools)
install(FILES ../doc/ComplxChangeLog.txt DESTINATION share/doc/complx-tools)
install(FILES ../doc/PattPatelAppA.pdf DESTINATION share/doc/complx-tools)
install(FILES ../doc/complx-tips.txt DESTINATION share/doc/complx-tools)
install(FILES ../complx.png DESTINATION share/icons)
install(FILES ../complx.desktop DESTINATION share/applications)
