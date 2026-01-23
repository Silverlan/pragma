// SPDX-FileCopyrightText: (c) 2023 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#ifdef _WIN32
#include <Windows.h>
#endif
#include <cstdio>

export module pragma.shared:console.spdlog_anycolor_sink;

import :console.output;
export import std.compat;

export namespace pragma::console {
	spdlog::details::console_mutex::mutex_t &get_mutex();
	// Based on wincolor_sink
	template<typename ConsoleMutex>
	class anycolor_sink : public spdlog::sinks::sink {
	  public:
#ifdef _WIN32
		anycolor_sink(void *out_handle, spdlog::color_mode mode);
#else
		anycolor_sink(FILE *target_file, spdlog::color_mode mode);
#endif
		~anycolor_sink() override;

		anycolor_sink(const anycolor_sink &other) = delete;
		anycolor_sink &operator=(const anycolor_sink &other) = delete;

		// change the color for the given level
		void set_color(spdlog::level::level_enum level, const std::string &color);
		void log(const spdlog::details::log_msg &msg) final override;
		void flush() final override;
		void set_pattern(const std::string &pattern) override final;
		void set_formatter(std::unique_ptr<spdlog::formatter> sink_formatter) override final;
		void set_color_mode(spdlog::color_mode mode);
	  protected:
		using mutex_t = typename ConsoleMutex::mutex_t;
#ifdef _WIN32
		void *out_handle_;
#else
		FILE *target_file_;
#endif
		mutex_t &mutex_;
		bool should_do_colors_;
		std::unique_ptr<spdlog::formatter> formatter_;
		std::array<spdlog::memory_buf_t, spdlog::level::n_levels> colors_;

		// print a range of formatted message to console
		void print_range_(const spdlog::memory_buf_t &formatted, size_t start, size_t end);

#ifdef _WIN32
		// in case we are redirected to file (not in console mode)
		void write_to_file_(const spdlog::memory_buf_t &formatted);
#endif

		void set_color_mode_impl(spdlog::color_mode mode);
	};

	template<typename ConsoleMutex>
	class anycolor_stdout_sink : public anycolor_sink<ConsoleMutex> {
	  public:
		explicit anycolor_stdout_sink(spdlog::color_mode mode = spdlog::color_mode::automatic);
	};

	template<typename ConsoleMutex>
	class anycolor_stderr_sink : public anycolor_sink<ConsoleMutex> {
	  public:
		explicit anycolor_stderr_sink(spdlog::color_mode mode = spdlog::color_mode::automatic);
	};

	using anycolor_stdout_sink_mt = anycolor_stdout_sink<spdlog::details::console_mutex>;
	using anycolor_stdout_sink_st = anycolor_stdout_sink<spdlog::details::console_nullmutex>;

	using anycolor_stderr_sink_mt = anycolor_stderr_sink<spdlog::details::console_mutex>;
	using anycolor_stderr_sink_st = anycolor_stderr_sink<spdlog::details::console_nullmutex>;

	using anycolor_color_sink_mt = anycolor_stdout_sink_mt;

#ifdef _WIN32
	template<typename ConsoleMutex>
	inline anycolor_sink<ConsoleMutex>::anycolor_sink(void *out_handle, spdlog::color_mode mode) : out_handle_(out_handle), mutex_(get_mutex()), formatter_(spdlog::details::make_unique<spdlog::pattern_formatter>())
	{

		set_color_mode_impl(mode);
	}
#else
	template<typename ConsoleMutex>
	inline anycolor_sink<ConsoleMutex>::anycolor_sink(FILE *target_file, spdlog::color_mode mode) : target_file_(target_file), mutex_(get_mutex()), formatter_(spdlog::details::make_unique<spdlog::pattern_formatter>())
	{

		set_color_mode_impl(mode);
	}
#endif

	template<typename ConsoleMutex>
	inline anycolor_sink<ConsoleMutex>::~anycolor_sink()
	{
		this->flush();
	}

	// change the color for the given level
	template<typename ConsoleMutex>
	void inline anycolor_sink<ConsoleMutex>::set_color(spdlog::level::level_enum level, const std::string &color)
	{
		std::lock_guard<mutex_t> lock(mutex_);
#ifdef SPDLOG_USE_STD_FORMAT
		colors_[static_cast<size_t>(level)] = color;
#else
		colors_[static_cast<size_t>(level)].append(color.data(), color.data() + color.size());
#endif
	}

	template<typename ConsoleMutex>
	void inline anycolor_sink<ConsoleMutex>::log(const spdlog::details::log_msg &msg)
	{
#ifdef _WIN32
		if(out_handle_ == nullptr || out_handle_ == INVALID_HANDLE_VALUE) {
			return;
		}
#endif

		std::lock_guard<mutex_t> lock(mutex_);
		msg.color_range_start = 0;
		msg.color_range_end = 0;
		spdlog::memory_buf_t formatted;
		formatter_->format(msg, formatted);
		if(should_do_colors_ && msg.color_range_end > msg.color_range_start) {
			// before color range
			print_range_(formatted, 0, msg.color_range_start);
			// in color range
			auto &col = colors_[static_cast<size_t>(msg.level)];
			print_range_(col, 0, col.size());
			print_range_(formatted, msg.color_range_start, msg.color_range_end);
			// reset to orig colors
#ifdef SPDLOG_USE_STD_FORMAT
			auto &reset = Con::COLOR_RESET;
#else
			spdlog::memory_buf_t reset {};
			reset.append(Con::COLOR_RESET.data(), Con::COLOR_RESET.data() + Con::COLOR_RESET.size());
#endif
			print_range_(reset, 0, reset.size());
			print_range_(formatted, msg.color_range_end, formatted.size());
		}
		else // print without colors if color range is invalid (or color is disabled)
		{
#ifdef _WIN32
			write_to_file_(formatted);
#else
			print_range_(formatted, 0, formatted.size());
#endif
		}
	}

	template<typename ConsoleMutex>
	void inline anycolor_sink<ConsoleMutex>::flush()
	{
#ifndef _WIN32
		std::lock_guard<mutex_t> lock(mutex_);
		fflush(target_file_);
#endif
	}

	template<typename ConsoleMutex>
	void inline anycolor_sink<ConsoleMutex>::set_pattern(const std::string &pattern)
	{
		std::lock_guard<mutex_t> lock(mutex_);
		formatter_ = std::unique_ptr<spdlog::formatter>(new spdlog::pattern_formatter(pattern));
	}

	template<typename ConsoleMutex>
	void inline anycolor_sink<ConsoleMutex>::set_formatter(std::unique_ptr<spdlog::formatter> sink_formatter)
	{
		std::lock_guard<mutex_t> lock(mutex_);
		formatter_ = std::move(sink_formatter);
	}

	template<typename ConsoleMutex>
	void inline anycolor_sink<ConsoleMutex>::set_color_mode(spdlog::color_mode mode)
	{
		std::lock_guard<mutex_t> lock(mutex_);
		set_color_mode_impl(mode);
	}

	template<typename ConsoleMutex>
	void inline anycolor_sink<ConsoleMutex>::set_color_mode_impl(spdlog::color_mode mode)
	{
#ifdef _WIN32
		if(mode == spdlog::color_mode::automatic) {
			// should do colors only if out_handle_  points to actual console.
			DWORD console_mode;
			bool in_console = ::GetConsoleMode(static_cast<HANDLE>(out_handle_), &console_mode) != 0;
			should_do_colors_ = in_console;
		}
		else {
			should_do_colors_ = mode == spdlog::color_mode::always ? true : false;
		}
#else
		switch(mode) {
		case spdlog::color_mode::always:
			should_do_colors_ = true;
			return;
		case spdlog::color_mode::automatic:
			should_do_colors_ = spdlog::details::os::in_terminal(target_file_) && spdlog::details::os::is_color_terminal();
			return;
		case spdlog::color_mode::never:
			should_do_colors_ = false;
			return;
		default:
			should_do_colors_ = false;
		}
#endif
	}

	// print a range of formatted message to console
	template<typename ConsoleMutex>
	void inline anycolor_sink<ConsoleMutex>::print_range_(const spdlog::memory_buf_t &formatted, size_t start, size_t end)
	{
#ifdef _WIN32
		if(end > start) {
			auto size = static_cast<DWORD>(end - start);
			auto ignored = ::WriteConsoleA(static_cast<HANDLE>(out_handle_), formatted.data() + start, size, nullptr, nullptr);
			(void)(ignored);
		}
#else
		fwrite(formatted.data() + start, sizeof(char), end - start, target_file_);
#endif
	}

#ifdef _WIN32
	template<typename ConsoleMutex>
	void inline anycolor_sink<ConsoleMutex>::write_to_file_(const spdlog::memory_buf_t &formatted)
	{
		auto size = static_cast<DWORD>(formatted.size());
		DWORD bytes_written = 0;
		auto ignored = ::WriteFile(static_cast<HANDLE>(out_handle_), formatted.data(), size, &bytes_written, nullptr);
		(void)(ignored);
	}
#endif

#ifdef _WIN32
	// anycolor_stdout_sink
	template<typename ConsoleMutex>
	inline anycolor_stdout_sink<ConsoleMutex>::anycolor_stdout_sink(spdlog::color_mode mode) : anycolor_sink<ConsoleMutex>(::GetStdHandle(STD_OUTPUT_HANDLE), mode)
	{
	}

	// anycolor_stderr_sink
	template<typename ConsoleMutex>
	inline anycolor_stderr_sink<ConsoleMutex>::anycolor_stderr_sink(spdlog::color_mode mode) : anycolor_sink<ConsoleMutex>(::GetStdHandle(STD_ERROR_HANDLE), mode)
	{
	}
#else
	// ansicolor_stdout_sink
	template<typename ConsoleMutex>
	inline anycolor_stdout_sink<ConsoleMutex>::anycolor_stdout_sink(spdlog::color_mode mode) : anycolor_sink<ConsoleMutex>(stdout, mode)
	{
	}

	// ansicolor_stderr_sink
	template<typename ConsoleMutex>
	inline anycolor_stderr_sink<ConsoleMutex>::anycolor_stderr_sink(spdlog::color_mode mode) : anycolor_sink<ConsoleMutex>(stderr, mode)
	{
	}
#endif
};
