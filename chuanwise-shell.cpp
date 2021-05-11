/*
* Shell
* OS homework
* copyright 2021 by Chuanwise
* Github: https://github.com/Chuanwise/chuanwise-shell
*/
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <fstream>
#include <set>
#include <stack>
#include <optional>
#include <unistd.h>
#include <pwd.h>
#include <sstream>
#include <assert.h>

constexpr int MAX_BUFFER = 1024;
constexpr const char* AUTHOR = "Chuanwise";
constexpr const char* GITHUB = "https://github.com/Chuanwise/chuanwise-shell";

std::string get_working_path() {
    static char working_path[MAX_BUFFER];
    getcwd(working_path, MAX_BUFFER);
    if (working_path) {
        return working_path;
    } else {
        return "unknown-path";
    }
}

std::string get_host_name() {
    static char host_name[MAX_BUFFER];
    gethostname(host_name, MAX_BUFFER);
    if (host_name) {
        return host_name;
    } else {
        return "unknown-host";
    }
}

std::string get_user_name() {
    // user name
    auto pwuid = getpwuid(getuid());
    if (pwuid) {
        return pwuid->pw_name;
    } else {
        return "unknown-user";
    }
}

/*
* 和 Shell 有关的异常
*/
class ShellException : virtual public std::exception {
public:
	explicit ShellException(std::string para_message) : message(para_message.c_str()) {}

	const char* what() const noexcept override {
		return message.c_str();
	}
protected:
	std::string message;
};

/*
* 表示 Shell 的一次有效输入
*/
class Command {
public:
	explicit Command(std::vector<std::string> tokens) {
		if (!tokens.empty()) {
			set_head(tokens[0]);
			for (int index = 1; index < tokens.size(); index++) {
				add_argument(tokens[index]);
			}
		}
	}

	explicit Command(std::string head) {
		set_head(head);
	}

	std::vector<std::string> get_arguments() {
		return arguments;
	}

	void add_argument(std::string argument) {
		arguments.emplace_back(argument);
	}

	void set_head(std::string head) {
		this->head = head;
	}

	std::string get_head() {
		return head;
	}

	std::string to_original_string() {
		std::string result = head;
		std::string remain_arguments = get_remain_arguments();
		if (!remain_arguments.empty()) {
			return result + " " + remain_arguments;
		} else {
			return result;
		}
	}

	std::string get_remain_arguments(int begin = 0) {
		if (begin >= arguments.size()) {
			return "";
		}

		std::string result = arguments[begin];
		for (int index = begin + 1; index < arguments.size(); index++) {
			auto& argument = arguments[index];
			if (argument.find(" ") == std::string::npos) {
				result += " " + argument;
			} else {
				result += " \"" + argument + "\"";
			}
		}
		return result;
	}

	std::ios::openmode get_err_out_mode() {
		return err_out_mode;
	}

	void set_err_out_mode(std::ios::openmode err_out_mode) {
		this->err_out_mode = err_out_mode;
	}

	std::ios::openmode get_out_mode() {
		return out_mode;
	}

	void set_out_mode(std::ios::openmode out_mode) {
		this->out_mode = out_mode;
	}

	void set_in_redirection(std::string in_redirection) {
		this->in_redirection = in_redirection;
	}

	std::string get_in_redirection() {
		return in_redirection;
	}

	void set_out_redirection(std::string out_redirection) {
		this->out_redirection = out_redirection;
	}

	std::string get_out_redirection() {
		return out_redirection;
	}

	void set_err_redirection(std::string err_redirection) {
		this->err_redirection = err_redirection;
	}

	std::string get_err_redirection() {
		return err_redirection;
	}
protected:
	std::string head;
	std::vector<std::string> arguments;

	std::string in_redirection;
	std::string err_redirection;
	std::string out_redirection;

	std::ios::openmode out_mode;
	std::ios::openmode err_out_mode;
};

/*
* Token 解析器
*/
class Spliter {
public:
	void set_space(std::string space) {
		this->space = space;
	}

	void set_board(std::string board) {
		this->board = board;
	}

	void add_keyword(std::string keyword) {
		keywords.insert(keyword);
	}

	std::set<std::string> get_keywords() {
		return keywords;
	}

	std::vector<std::string> split(std::string input) {
		char* ptr = nullptr;
		std::vector<std::string> result;

		enum class State {
			SPACING,
			ARGUMENT,
			BOARD_ARGUMENT,
		};

		State state = State::SPACING;
		std::string buffer;

		std::string last_keyword;
		bool is_keyword_prefix = false;
		bool is_keyword = false;

		for (int index = 0; index < input.length(); index++) {
			auto& ch = input[index];

			switch (state) {
			case State::SPACING:
				// 空字符不改变状态
				if (space.find(ch) != std::string::npos) {
					continue;
				}
				// 特殊范围标记的参数，例如 "argument with spaces"
				if (board.find(ch) != std::string::npos) {
					state = State::BOARD_ARGUMENT;
					continue;
				}
				// 其他则是普通参数
				buffer.push_back(ch);
				state = State::ARGUMENT;
				break;
			case State::ARGUMENT:
				// 空字符或是关键字则保存当前输入，并迁移到空字符状态
				if (space.find(ch) != std::string::npos) {
					state = State::SPACING;
					result.emplace_back(buffer);
					buffer.clear();
					continue;
				}
				// 如果是关键词，可能会有相同前缀的关键词例如 >> 和 >
				for (auto& keyword : keywords) {
					if (buffer.find(keyword) == 0) {
						is_keyword_prefix = true;
						// 之前的是 keyword 或当前的是
						is_keyword = is_keyword || keyword.length() == buffer.length();
					}
				}

				// 如果找到有前缀和当前缓存区相同的，可能就是上述的特殊情况
				if (is_keyword_prefix) {
					// 如果也找到了关键词，那就先继续往下寻找，因为还可能有更长的关键词
					if (is_keyword) {
						last_keyword = buffer;
						is_keyword = false;
					} else if (!last_keyword.empty()) {
						// 否则就是找到了，回滚到最后一次，也就是最长的 buffer
						// 多 -1 是为了消除循环结束 + 1 的影响
						index -= buffer.length() - last_keyword.length() + 1;
						buffer = last_keyword;
						last_keyword.clear();

						result.emplace_back(buffer);
						buffer.clear();
						is_keyword = false;
						is_keyword_prefix = false;
						continue;
					}
				}

				buffer.push_back(ch);
				break;
			case State::BOARD_ARGUMENT:
				// 特殊范围标记符号时退出
				if (board.find(ch) != std::string::npos) {
					state = State::SPACING;
					result.emplace_back(buffer);
					buffer.clear();
					continue;
				}
				buffer.push_back(ch);
				break;
			default:
				throw ShellException("an internal error occurs: illegal state for argument parse FSM");
			}
		}

		// 最后一个参数可能因为循环到结尾而没有纳入
		if (!buffer.empty()) {
			if (last_keyword.empty()) {
				result.emplace_back(buffer);
			} else {
				result.emplace_back(last_keyword);
				result.emplace_back(buffer.substr(last_keyword.length() - 1));
			}
		}
		return result;
	}
protected:
	std::string space = " \n\t\r";

	std::string board = "\"";

	std::set<std::string> keywords;
};

/*
* Shell
*/
class Shell {
public:
	bool on_command(std::string input) {
		if (input.empty()) {
			return true;
		}
		restore_buffers();

		auto split_result = spliter.split(input);
		auto contexts = to_commands(split_result);

		if (contexts.size() == 1) {
			on_command(contexts[0]);
		} else {
			std::stringbuf out_string_buffer;
			std::stringbuf in_string_buffer;

			for (auto p = contexts.begin() + 1; p != contexts.end(); p++) {
				// 设置它们之间的管道关系
				auto& last_command = *(p - 1);
				auto& cur_command = *p;

				// 不断交换 in 和 out
				if ((p - contexts.begin()) & 1) {
					out_buffer = &in_string_buffer;
				} else {
					out_buffer = &out_string_buffer;
				}

				// 设置管道信息
				// in 不能被重定向
				auto cur_in_redirection = cur_command.get_in_redirection();
				if (!cur_in_redirection.empty()) {
					throw ShellException("input of command \"" + cur_command.to_original_string() +
						"\" already be redirected to output of last command: \"" + last_command.to_original_string() + "\"");
				}

				// out 重定向到下一个的 in
				auto last_out_redirection = last_command.get_out_redirection();
				if (!last_out_redirection.empty()) {
					throw ShellException("output of command \"" + last_command.to_original_string() +
						"\" already be redirected to input of next command: \"" + cur_command.to_original_string() + "\"");
				}

				// err 重定向的话照常
				auto last_err_redirection = last_command.get_err_redirection();
				std::ofstream err_file;
				if (!last_err_redirection.empty()) {
					err_file.open(last_err_redirection, last_command.get_err_out_mode());
					if (err_file.is_open()) {
						err_buffer = err_file.rdbuf();
					} else {
						throw ShellException("can not open the error output stream of command \"" + last_command.to_original_string() +
							"\": \"" + last_err_redirection + "\"");
					}
				}

				on_command(last_command);

				// 不断交换 in 和 out
				if ((p - contexts.begin()) & 1) {
					in_buffer = &in_string_buffer;
				} else {
					in_buffer = &out_string_buffer;
				}

				// 如果有 err 重定向，保存文件
				if (err_file.is_open()) {
					err_file.close();
				}
			}

			// 设置最后一次的 out
			auto& back_command = contexts.back();
			auto back_in_redirection = back_command.get_in_redirection();
			if (!back_in_redirection.empty()) {
				throw ShellException("input of command \"" + back_command.to_original_string() +
					"\" already be redirected to output of last command: \"" + contexts[contexts.size() - 2].to_original_string() + "\"");
			}

			out_buffer = cout_buffer;
			on_command(back_command);
		}
		restore_buffers();
		return true;
	}

	void on_command(Command command) {
		// 输入重定向
		auto in_redirection = command.get_in_redirection();
		auto out_redirection = command.get_out_redirection();
		auto err_redirection = command.get_err_redirection();

		std::ifstream in_file;
		if (!in_redirection.empty()) {
			in_file.open(in_redirection, std::ios::in);
			if (in_file.is_open()) {
				in_buffer = in_file.rdbuf();
			}
		}

		std::ifstream out_file;
		if (!out_redirection.empty()) {
			if (out_redirection == "&2") {
				out_buffer = err_buffer;
			} else if (out_redirection != "&1") {
				out_file.open(out_redirection, command.get_out_mode());
				if (out_file.is_open()) {
					out_buffer = out_file.rdbuf();
				}
			} else {
				throw ShellException("meaningless output redirection: out -> out");
			}
		}

		std::ifstream err_file;
		if (!err_redirection.empty()) {
			if (err_redirection == "&1") {
				err_buffer = in_buffer;
			} else if (err_redirection != "&2") {
				err_file.open(err_redirection, command.get_err_out_mode());
				if (err_file.is_open()) {
					err_buffer = err_file.rdbuf();
				}
			} else {
				throw ShellException("meaningless output redirection: err -> err");
			}
		}

		execute_in_current_env(command);

		// 关闭重定向的文件
		if (in_file.is_open()) {
			in_file.close();
		}
		if (out_file.is_open()) {
			out_file.close();
		}
		if (err_file.is_open()) {
			err_file.close();
		}

		restore_buffers();
	}

	void execute_in_current_env(Command command) {
		// 保存老值
		auto elder_in_buffer = in_buffer;
		auto elder_out_buffer = out_buffer;
		auto elder_err_buffer = err_buffer;

		// 重定向输入输出
		std::cin.rdbuf(in_buffer);
		std::cout.rdbuf(out_buffer);
		std::cerr.rdbuf(err_buffer);

		// 执行指令
		auto executor = executors.find(command.get_head());
		if (executor == executors.end()) {
			on_unknown_command(command);
		} else {
			executor->second(command);
		}

		// 恢复输入输出
		std::cin.rdbuf(elder_in_buffer);
		std::cout.rdbuf(elder_out_buffer);
		std::cerr.rdbuf(elder_err_buffer);
	}

	void on_command(Command command, std::streambuf* cur_in_buffer, std::streambuf* cur_out_buffer, std::streambuf* cur_err_buffer) {
		// 重定向输入输出
		std::cin.rdbuf(cur_in_buffer);
		std::cout.rdbuf(cur_out_buffer);
		std::cerr.rdbuf(cur_err_buffer);

		// 执行指令
		execute_in_current_env(command);

		// 恢复输入输出
		std::cin.rdbuf(in_buffer);
		std::cout.rdbuf(out_buffer);
		std::cerr.rdbuf(err_buffer);
	}

	virtual void on_unknown_command(Command command) = 0;

	bool has_command(std::string head) {
		return executors.find(head) != executors.end();
	}

	bool register_command(std::string head, std::function<void(Command)> executor) {
		if (has_command(head)) {
			return false;
		} else {
			executors[head] = executor;
			return true;
		}
	}

	Spliter& get_spliter() {
		return spliter;
	}

	void set_spliter(Spliter spliter) {
		this->spliter = spliter;
	}

	std::vector<Command> to_commands(std::vector<std::string> tokens) {
		std::vector<Command> result;
		std::vector<std::string> cur_tokens;

		for (auto& token : tokens) {
			if (token == "|") {
				result.emplace_back(to_single_command(cur_tokens));
				cur_tokens.clear();
			} else {
				cur_tokens.emplace_back(token);
			}
		}
		if (!cur_tokens.empty()) {
			result.emplace_back(to_single_command(cur_tokens));
		}
		return result;
	}

	Command to_single_command(std::vector<std::string> tokens) {
		enum class State {
			ARGUMENT,
			I_REDIRECTION,
			ERR_REDIRECTION,
			O_REDIRECTION,
		};

		Command result(tokens[0]);
		State state = State::ARGUMENT;

		for (int index = 1; index < tokens.size(); index ++) {
			auto& token = tokens[index];
			switch (state) {
			case State::ARGUMENT:
				// 输出重定向
				if (token == ">" || token == ">>" || token == "1>" || token == "1>>") {
					// 追加和新增两种区分
					if (token.find(">>") != std::string::npos) {
						result.set_out_mode(std::ios::out | std::ios::app);
					} else {
						result.set_out_mode(std::ios::out | std::ios::trunc);
					}
					state = State::O_REDIRECTION;
					continue;
				}

				// 错误信息输出重定向
				if (token == "2>" || token == "2>>") {
					// 追加和新增两种区分
					if (token.find(">>") != std::string::npos) {
						result.set_err_out_mode(std::ios::out | std::ios::app);
					} else {
						result.set_err_out_mode(std::ios::out | std::ios::trunc);
					}
					state = State::ERR_REDIRECTION;
					continue;
				}

				// 输入重定向
				if (token == "<") {
					state = State::I_REDIRECTION;
					continue;
				}
				result.add_argument(token);
				break;
			case State::I_REDIRECTION:
				if (result.get_in_redirection().empty()) {
					result.set_in_redirection(token);
					state = State::ARGUMENT;
				} else {
					throw ShellException("input redirection already set!");
				}
				break;
			case State::O_REDIRECTION:
				if (result.get_out_redirection().empty()) {
					result.set_out_redirection(token);
					state = State::ARGUMENT;
				} else {
					throw ShellException("output redirection already set!");
				}
				break;
			case State::ERR_REDIRECTION:
				if (result.get_err_redirection().empty()) {
					result.set_err_redirection(token);
					state = State::ARGUMENT;
				} else {
					throw ShellException("error redirection already set!");
				}
				break;
			default:
				throw ShellException("an internal error occurs: illegal state for command parse FSM");
			}
		}
		return result;
	}

	void restore_buffers() {
		if (in_buffer != cin_buffer) {
			in_buffer = cin_buffer;
			std::cin.rdbuf(cin_buffer);
		}
		if (out_buffer != cout_buffer) {
			out_buffer = cout_buffer;
			std::cout.rdbuf(cout_buffer);
		}
		if (err_buffer != cerr_buffer) {
			err_buffer = cerr_buffer;
			std::cerr.rdbuf(cerr_buffer);
		}
	}

	const std::unordered_map<std::string, std::function<void(Command)>>& get_executors() {
		return executors;
	}
protected:
	std::unordered_map<std::string, std::function<void(Command)>> executors;
	Spliter spliter;

	std::streambuf* in_buffer = cin_buffer;
	std::streambuf* out_buffer = cout_buffer;
	std::streambuf* err_buffer = cerr_buffer;

	std::streambuf* const cin_buffer = std::cin.rdbuf();
	std::streambuf* const cout_buffer = std::cout.rdbuf();
	std::streambuf* const cerr_buffer = std::cerr.rdbuf();
};

/*
* Shell 的一个子类，主要是自动注册了作业里要求的 ls 之类的指令
*/
class LinuxShell : virtual public Shell {
public:
	LinuxShell() {
		initialize();
	}

	void on_unknown_command(Command command) override {
		std::cerr << "bash: " << command.get_head() << ": No such command, press \"help\" to get more details." << std::endl;
	}
private:
	void initialize() {
		initialize_spliter();
		register_base_commands();
	}

	void initialize_spliter() {
		get_spliter().add_keyword(">");
		get_spliter().add_keyword(">>");
		get_spliter().add_keyword("1>");
		get_spliter().add_keyword("1>>");
		get_spliter().add_keyword("2>");
		get_spliter().add_keyword("2>>");
		get_spliter().add_keyword("&1");
		get_spliter().add_keyword("&2");
	}

	void register_base_commands() {
		/*
		* i. cd <directory>
		* Change the current default directory to <directory>.
		* If the <directory> argument is not present, report the current directory.
		* If the directory does not exist an appropriate error should be reported.
		* This command should also change the PWD environment variable.
		*/
		register_command("cd", [](Command command) {
			std::string path = command.get_remain_arguments();
			if (chdir(path.c_str()) == -1 && chdir((get_working_path() + "/" + path).c_str()) == -1) {
				std::cout << "bach: cd: " << path << ": No such file or directory" << std::endl;
			}
		});

		/*
		* cat
		* show the content of a text file
		*/
		register_command("cat", [](Command command) {
			std::string file_name = command.get_remain_arguments();
			if (file_name.empty()) {
				char ch;
				while ((ch = std::cin.get()) != EOF) {
					std::cout << ch;
				}
			} else {
				std::ifstream file;
				file.open(file_name);
				if (file.is_open()) {
					char ch;
					while ((ch = file.get()) != EOF) {
						std::cout << ch;
					}
					file.close();
				} else {
					std::cout << "cat: " << file_name << ": No such file or directory" << std::endl;
				}
			}
		});

		/*
		* vi. help
		* Display the user manual using the more filter.
		*/
		register_command("help", [](Command command) {
			std::ifstream file;
			file.open("help.txt");
			if (file.is_open()) {
				char ch;
				while ((ch = file.get()) != EOF) {
					std::cout << ch;
				}
				file.close();
			} else {
				std::cout << "Can not open the help document, see it in github: " << std::endl;
			}
		});

		/*
		* v. echo <comment>
		* Display <comment> on the display followed by a new line (multiple spaces/tabs may be reduced to a single space).
		*/
		register_command("echo", [](Command command) {
			std::cout << command.get_remain_arguments() << std::endl;
		});

		/*
		* iv. environ
		* List all the environment strings.
		*/
		register_command("environ", [](Command command) {
			std::string variable_name = command.get_remain_arguments();
			if (variable_name.empty()) {
				system("set");
			} else {
				char* env_val = getenv(variable_name.c_str());
				if (env_val) {
					std::cout << variable_name << " = " << env_val << std::endl;
				} else {
					std::cerr << "No such environment variable: " << variable_name << std::endl;
				}
			}
		});

		/*
		* vii. pause
		* Pause operation of the linux_shell until 'Enter' is pressed.
		*/
		register_command("pause", [](Command command) {
			std::cout << "press enter to continue" << std::endl;
			getchar();
			fflush(stdin);
		});
		/*
		* viii. quit - Quit the linux_shell.
		*/
		register_command("quit", [](Command command) {
			std::cout << "Good bye!" << std::endl;
			exit(0);
		});
	
		/*
		* ls
		*/
		register_command("ls", [](Command command) {
			system(command.to_original_string().c_str());
		});
	}
};

int main() {
	static const std::string LOGO = std::string() +
		" _____ _    _ _____ _          _ _ \n" +
		"/  __ \\ |  | /  ___| |        | | |\n" +
		"| /  \\/ |  | \\ `--.| |__   ___| | |\n" +
		"| |   | |/\\| |`--. \\ '_ \\ / _ \\ | |\n" +
		"| \\__/\\  /\\  /\\__/ / | | |  __/ | |\n" +
		" \\____/\\/  \\/\\____/|_| |_|\\___|_|_|";

	std::cout << LOGO << std::endl << std::endl
		<< "CWShell @" << AUTHOR << std::endl
		<< "Github: " << GITHUB << std::endl
		<< "Welcome to star!" << std::endl << std::endl;

	LinuxShell linux_shell;

	linux_shell.register_command("printerr", [](Command command) {
		std::cout << "cout" << std::endl;
		std::cerr << "cerr" << std::endl;
	});

	linux_shell.register_command("repeat", [](Command command) {
		auto arguments = command.get_remain_arguments();
		if (!arguments.empty()) {
			std::cout << "arguments: \"" << arguments << "\"" << std::endl;
		}
		std::string input;
		std::getline(std::cin, input);
		std::cout << "your input is: \"" << input << "\"" << std::endl;
	});

	std::string input;
	while (!feof(stdin)) {
		std::cout << get_user_name() << "@" << get_host_name() << ":" << get_working_path() << "$ ";
		std::getline(std::cin, input);

		try {
			linux_shell.on_command(input);
		} catch (ShellException& exception) {
			std::cerr << "ERROR: " << exception.what() << std::endl;
		}
	}
}