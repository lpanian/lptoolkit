#include "toolkit/cmdhelper.hh"
#include <cstdio>
#include <cstring>
#include <algorithm>
#include "toolkit/dynary.hh"

namespace lptk
{
	CmdLineOpt* CmdLineOpt::s_opts = nullptr;

	class HelpCmdLineOpt : public CmdLineOpt
	{
	public:
		HelpCmdLineOpt() : CmdLineOpt("help", "display help", 0) {}

		bool Parse(int, char**) override
		{
			lptk::DynAry<const CmdLineOpt*> all;
			const CmdLineOpt* opt = s_opts;
			while(opt)
			{
				all.push_back(opt);
				opt = opt->GetNext();
			}

			std::sort(all.begin(), all.end(), [](const CmdLineOpt* left, const CmdLineOpt* right){
					return StrCaseCmp(left->GetLong(), right->GetLong()) < 0;
					});

			printf(
					"The following options are available:\n");
			for(auto opt : all)
			{
				const auto leftColMax = 30;
				const auto longLen = strlen(opt->GetLong());
				const auto argSpec = opt->GetNumArgs() > 1 ? " args..." : 
					opt->GetNumArgs() > 0 ? " arg" : "";
				const auto argSpecLen = strlen(argSpec);
				const int leftColSize = std::max(1, static_cast<int>(leftColMax - longLen - argSpecLen));
				printf(
				"    --%s%s%*s%s\n", opt->GetLong(), argSpec, leftColSize, "", opt->GetDesc());
			}
			s_helpRan = true;
			return true;
		}

		bool HelpRan() const { return s_helpRan; }

	private:
		static bool s_helpRan;
	};
	static HelpCmdLineOpt s_helpCmdLineOpt;
	bool HelpCmdLineOpt::s_helpRan = false;


	CmdLineOpt::CmdLineOpt(const char* optLong, const char* optDesc, int numArgs)
		: m_long(optLong)
		, m_desc(optDesc)
		, m_numArgs(numArgs)
		, m_prev(nullptr)
		, m_next(nullptr)
	{
		if(s_opts)
		{
			s_opts->m_prev = this;
			m_next = s_opts;
		}
		s_opts = this;
	}
		
	CmdLineOpt::~CmdLineOpt()
	{
		if(m_prev)
			m_prev->m_next = m_next;
		if(m_next)
			m_next->m_prev = m_prev;
	}
		
	CmdLineOpt* CmdLineOpt::FindByCommand(const char* name)
	{
		if(name[0] && name[0] == '-')
		{
			if(name[1] && name[1] == '-')
			{
				// search long names
				CmdLineOpt* cur = s_opts;
				while(cur)
				{
					if(StrCaseEqual(cur->m_long, &name[2]))
						return cur;
					cur = cur->m_next;
				}
			}
			else
			{
				CmdLineOpt* best = nullptr;
				bool multiple = false;
				// try to match short names with long names

				const auto nameLen = static_cast<unsigned int>(strlen(&name[1]));
				CmdLineOpt* cur = s_opts;
				while(cur)
				{
					if(StrNCaseEqual(cur->m_long, &name[1], nameLen))
					{
						if(!multiple && !best)
							best = cur;
						else
							multiple = true;
						
						if(multiple)
						{
							if(best)
							{
								printf("Ambiguous command \"%s\", did you mean one of these: \n", name);
								printf(
										"    --%s\n", best->m_long);
								best = nullptr;
							}
							printf(
										"    --%s\n", cur->m_long);
						}
					}
					cur = cur->m_next;
				}
				return best;
			}
		}
		return nullptr;
	}
	
	bool CmdLineOpt::HelpRan() 
	{ 
		return s_helpCmdLineOpt.HelpRan(); 
	}

	bool CmdLineOpt::ProcessArguments(int argc, char** argv)
	{
		CmdLineOpt* restOpt = s_opts;
		while(restOpt)
		{
			if(restOpt->m_numArgs == -1)
				break;
			restOpt = restOpt->m_next;
		}

		int remainingIndex = 1;
		for(int i = 1; i < argc; ++i)
		{
			const char *cmd = argv[i];

			CmdLineOpt* cur = FindByCommand(cmd);
			if(cur)
			{
				int j = i + 1;
				while(j < argc && argv[j][0] != '-') ++j;

				const int numArgs = j - i - 1;
				if(cur->m_numArgs <= numArgs)
				{
					if(cur == &s_helpCmdLineOpt)
					{
						printf("usage: %s [options]", argv[0]);
						if(restOpt)
							printf(" [%s]", restOpt->GetLong());
						printf("\n");
					}

					if(!cur->Parse(numArgs, &argv[i+1]))
					{
						printf("failed to process command line argument \"%s\"\n", cmd);
						return false;
					}
				}
				else
				{
					printf("Not enough arguments for option %s. It requires %d parameters.\n",
							cur->m_long, cur->m_numArgs);
				}
				i = j - 1;
				if(cur == &s_helpCmdLineOpt)
					return true;
				remainingIndex = i + 1;
			}
			else if(cmd[0] == '-')
			{
				printf("WARN: Ignoring unrecognized option: %s\n", cmd);
				remainingIndex = i + 1;
			}
		}

		if(restOpt && remainingIndex < argc)
		{
			const int count = argc - remainingIndex;
			if(!restOpt->Parse(count, &argv[remainingIndex]))
				return false;
		}
		return true;
	}

}

