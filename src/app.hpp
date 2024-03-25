#pragma once

class App
{
bool show_demo_window {true};
bool showMainViewportMenuBar {true};

public:
	~App() {}
	 App() {}

	App           (      App&&) = delete;
	App           (const App& ) = delete;
	App  operator=(      App&&) = delete;
	App& operator=(const App& ) = delete;

	// Main _apping_ functor (function) of this `App`
	void operator()(void);

private:
	void MainViewportMenuBar();
	void MainWindow();
};
