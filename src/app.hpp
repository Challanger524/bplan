#pragma once

class App
{
bool show_demo_window {true};

public:
	~App() {}
	 App() {}

	App           (      App&&) = delete;
	App           (const App& ) = delete;
	App  operator=(      App&&) = delete;
	App& operator=(const App& ) = delete;

	// Main functor (function) of this application
	void operator()(void);
};
