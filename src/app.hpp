#pragma once

class App
{

public:
	~App() {}
	 App() {}

	App           (const App& ) = delete;
	App           (      App&&) = delete;
	App& operator=(const App& ) = delete;
	App& operator=(      App&&) = delete;

	// \>\> Main loop _apping_ functor for `App` <<
	void operator()(void);

private:
	void MainMenuBar();
	void MainWindow ();

public:
#ifdef TESTING
	//friend class Test;
	//Test test{this};
#endif
};
