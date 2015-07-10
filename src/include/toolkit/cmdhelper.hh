#pragma once
#ifndef INCLUDED_toolkit_cmdhelper_hh
#define INCLUDED_toolkit_cmdhelper_hh

#include <cstdlib>
#include <type_traits>

namespace lptk
{

	////////////////////////////////////////////////////////////////////////////////
	class CmdLineOpt
	{
	public:
		////////////////////////////////////////
		CmdLineOpt(const char* optLong, const char* optDesc, int numArgs);
		virtual ~CmdLineOpt();
		
		////////////////////////////////////////
		const char* GetLong() const { return m_long; }
		const char* GetDesc() const { return m_desc; }
		int GetNumArgs() const { return m_numArgs; }
		const CmdLineOpt* GetPrev() const { return m_prev; }
		const CmdLineOpt* GetNext() const { return m_next; }

		////////////////////////////////////////
		virtual bool Parse(int argc, char** argv) = 0;

		////////////////////////////////////////
		static bool ProcessArguments(int argc, char** argv);
		static bool HelpRan();
	protected:
		////////////////////////////////////////
		static CmdLineOpt* FindByCommand(const char* name);

		////////////////////////////////////////
		static CmdLineOpt* s_opts;

		////////////////////////////////////////
		const char* m_long;
		const char* m_desc;
		int m_numArgs;
		CmdLineOpt* m_prev;
		CmdLineOpt* m_next;
	};

	////////////////////////////////////////////////////////////////////////////////
	template<class T>
	class NumericCmdLineOpt : public CmdLineOpt
	{
	public:
		static_assert(std::is_arithmetic<T>::value, "NumericCmdLineOpt template parameter is not arithmetic");
		NumericCmdLineOpt(const char* name, const char* desc, T* target)
			: CmdLineOpt(name, desc, 1)
			, m_target(target)
		{}

		bool Parse(int argc, char** argv) override
		{
			if(!m_target || argc < 1)
				return false;

			if(std::is_floating_point<T>::value)
			{
				*m_target = atof(argv[0]);
			}
			else if(std::is_integral<T>::value)
			{
				if(std::is_signed<T>::value)
				{
					*m_target = atoll(argv[0]);
				}
				else
				{
					*m_target = strtoull(argv[0], NULL, 10);
				}
			}
			else 
				return false;
			return true;
		}
	private:
		T* m_target;
	};

	////////////////////////////////////////////////////////////////////////////////
	template<class Fn>
	class GenericCmdLineOpt : public CmdLineOpt
	{
	public:
		GenericCmdLineOpt(const char* name, const char* desc, int numArgs, Fn&& fn)
			: CmdLineOpt(name, desc, numArgs)
			, m_fn(std::move(fn))
		{}
		
		bool Parse(int argc, char** argv) override
		{
			return m_fn(argc, argv);
		}
	private:
		Fn m_fn;
	};

	template<class Fn>
	GenericCmdLineOpt<Fn> MakeOption(const char* name, const char* desc, int numArgs, Fn&& fn)
	{
		return GenericCmdLineOpt<Fn>(name, desc, numArgs, std::move(fn));
	}
	
	template<class Fn>
	GenericCmdLineOpt<Fn> MakeOption(const char* name, const char* desc, Fn&& fn)
	{
		return GenericCmdLineOpt<Fn>(name, desc, -1, std::move(fn));
	}
}

#endif

