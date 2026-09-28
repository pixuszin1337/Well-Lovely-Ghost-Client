#pragma once

#include "../sdk/includes.h"

namespace welllovely {
	class c_welllovely {
	private:
		JNIEnv* env;
		JavaVM* vm;

		jmethodID findclass_md;
		jobject classloader_obj;
	public:

		bool b_running;

		bool b_open;

		bool attach();

		void run();

		void dispose();

		void get_launchwrapper();

		void hook();

		JNIEnv* get_env() {
			JNIEnv* e = nullptr;
			if (vm)
				vm->GetEnv(reinterpret_cast<void**>(&e), JNI_VERSION_1_6);
			return e;
		}

		const auto get_vm() {
			return vm;
		}

		inline jclass find_class(const char* name) {
				jstring jname = get_env()->NewStringUTF(name);

				jclass cls = reinterpret_cast<jclass>(get_env()->CallObjectMethod(classloader_obj, findclass_md, jname));

				get_env()->DeleteLocalRef(jname);

				wrapper::handle_issue(name, cls);

				return cls;
		}

		inline jclass find_class_quiet(const char* name) {
			jstring jname = get_env()->NewStringUTF(name);

			jclass cls = reinterpret_cast<jclass>(get_env()->CallObjectMethod(classloader_obj, findclass_md, jname));

			get_env()->DeleteLocalRef(jname);

			if (get_env()->ExceptionCheck())
				get_env()->ExceptionClear();

			return cls;
		}
	};

	namespace hooks {

		using swap_buffers_fn = int(__stdcall*)(HDC);

		inline swap_buffers_fn oswap_buffers = nullptr;

		extern std::shared_ptr<wrapper::c_context> gl_context;

		int __stdcall swap_buffers_hk(HDC);

		inline WNDPROC original_wndproc = nullptr;

		inline HWND mc_hwnd = nullptr;

		LRESULT __stdcall wndproc_hk(HWND hwnd, UINT usermsg, WPARAM wparam, LPARAM lparam);
	}

	extern std::unique_ptr<c_welllovely> instance;
}
