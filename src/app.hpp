#pragma once

#include "test.hpp"
#include "app/data.hpp"

#include <memory>

class App
{
	std::unique_ptr<TaxUnit> root{};

public:
	~App() {}
	 App() {}

	App           (      App&&) = delete;
	App           (const App& ) = delete;
	App& operator=(      App&&) = delete;
	App& operator=(const App& ) = delete;

	// \>\> Main loop _apping_ functor of this `App` <<
	void operator()(void); // <- - - main loop of the `App`

private:
	void MainViewportMenuBar();
	void MainWindow();
	void DataTreeEditor();

public:
#if !defined(NDEBUG) || defined(TESTS_IN_APP)
	friend class Test;
	Test test{this};
#endif
};
