#include <Cocoa/Cocoa.h>

void leon_mac_init(){
	auto mainMenu = [NSMenu new];
	auto appMenuItem = [NSMenuItem new];
	auto appMenu = [NSMenu new];

	auto quitMenuItem = [[NSMenuItem alloc] initWithTitle:@"Quit 4coder"
											action:@selector(windowShouldClose:) // NOTE(Leon): Not calling terminate because there might be unsaved files which is handled by 4coder
											keyEquivalent:@"q"];
	[mainMenu addItem:appMenuItem];
	[appMenu addItem:quitMenuItem];
	[appMenuItem setSubmenu:appMenu];
	[NSApp setMainMenu:mainMenu];
}
