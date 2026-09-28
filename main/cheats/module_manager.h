#pragma once

#include "module.h"

namespace modules {

	class c_manager {
		std::vector<std::shared_ptr<c_module>> _modules;

		c_module* _binding = nullptr;
	public:
		template<class T, class... A>
		void add(A&&... args)
		{
			_modules.push_back(std::make_shared<T>(std::forward<A>(args)...));
		}

		void on_tick(std::shared_ptr<c_context> ctx)
		{

			for (auto& m : _modules)
			{
				if (m->enabled)
					m->on_tick(ctx);
				else if (m->was_enabled)
					m->on_disable(ctx);

				m->was_enabled = m->enabled;
			}
		}

		void on_key(int vk)
		{
			if (!vk)
				return;

			for (auto& m : _modules)
				if (m->keybind == vk)
					m->enabled = !m->enabled;
		}

		void begin_bind(c_module* m) { _binding = m; }

		bool is_binding() const { return _binding != nullptr; }

		const c_module* binding() const { return _binding; }

		void feed_key(int vk)
		{
			if (_binding)
			{
				_binding->keybind = (vk == VK_ESCAPE) ? 0 : vk;
				_binding = nullptr;
				return;
			}

			on_key(vk);
		}

		const std::vector<std::shared_ptr<c_module>>& get() const { return _modules; }
	};

	inline std::unique_ptr<c_manager> instance = nullptr;
}
