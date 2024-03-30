#pragma once

#include "app/data.hpp"

class App
{
	TaxUnit root{};
bool show_demo_window {false};
bool showMainViewportMenuBar {true};

public:
	~App() {}
	 App() {}

	App           (      App&&) = delete;
	App           (const App& ) = delete;
	App& operator=(      App&&) = delete;
	App& operator=(const App& ) = delete;

	// Main _apping_ functor (function) of this `App`
	void operator()(void);
	void Test() { root.Test(); }

	//void SwitchDisplayLevel() {root.SwitchDisplayLevel();}

private:
	void MainViewportMenuBar();
	void MainWindow();
	void DataTreeEditor();
};
