
/***************************************************Project Mutator****************************************************/
//-*-c++-*-
/*first line intentionally left blank.*/
/*the source code for SaferCPP's automatic refactoring of C/C++ arrays.*/
/*Copyright (C) 2017 Noah L.,Farzad Sadeghi

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.*/
/*code structure inspired by Eli Bendersky's tutorial on Rewriters.*/
/**********************************************************************************************************************/
/**********************************************************************************************************************/
/*included modules*/
/*Project Headers*/
#include "safercpp-arr.h"
#include "../mutator_aux.h"
/*Standard headers*/
#include <string>
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <locale>
/*Clang Headers*/
#include "clang/AST/AST.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Lex/Lexer.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "clang/Rewrite/Core/Rewriter.h"
/*LLVM Headers*/
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Function.h"
/**********************************************************************************************************************/
/*used namespaces*/
using namespace llvm;
using namespace clang;
using namespace clang::ast_matchers;
using namespace clang::driver;
using namespace clang::tooling;
/**********************************************************************************************************************/
static llvm::cl::OptionCategory MatcherSampleCategory("TBD");

cl::opt<bool> CheckSystemHeader("SysHeader", cl::desc("safercpp will run through System Headers"), cl::init(false), cl::cat(MatcherSampleCategory), cl::ZeroOrMore);
cl::opt<bool> MainFileOnly("MainOnly", cl::desc("safercpp will only report the results that reside in the main file"), cl::init(false), cl::cat(MatcherSampleCategory), cl::ZeroOrMore);
cl::opt<bool> SafeSubset("SafeSubset", cl::desc("safercpp will check for elements outside of the (memory) safe subset of the language"), cl::init(true), cl::cat(MatcherSampleCategory), cl::ZeroOrMore);
cl::opt<bool> ConvertToSCPP("ConvertToSCPP", cl::desc("safercpp will translate the source to a (memory) safe subset of the language"), cl::init(false), cl::cat(MatcherSampleCategory), cl::ZeroOrMore);
/**********************************************************************************************************************/

SourceRange nice_source_range(const SourceRange& sr, Rewriter &Rewrite) {
	SourceLocation SL = sr.getBegin();
	SourceLocation SLE = sr.getEnd();
	if (SL.isMacroID() || SLE.isMacroID()) {
		int q = 5;
	}
	SL = Devi::SourceLocationHasMacro(SL, Rewrite, "start");
	SLE = Devi::SourceLocationHasMacro(SLE, Rewrite, "end");
	return SourceRange(SL, SLE);
}

bool filtered_out_by_location(const ast_matchers::MatchFinder::MatchResult &MR, SourceLocation SL) {
	bool retval = false;
	if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, MR, SL)) {
		retval = true;
	} else if (!Devi::IsTheMatchInMainFile(MainFileOnly, MR, SL)) {
		retval = true;
	}
	return retval;
}

static std::string with_whitespace_removed(const std::string& str) {
	std::string retval = str;
	retval.erase(std::remove_if(retval.begin(), retval.end(), isspace), retval.end());
	return retval;
}

static std::string with_newlines_removed(const std::string& str) {
	std::string retval = str;
	auto riter1 = retval.rbegin();
	while (retval.rend() != riter1) {
		if ('\n' == *riter1) {
			auto riter2 = riter1;
			riter2++;
			retval.erase(riter1.base()--);
			while (retval.rend() != riter2) {
				/* look for and remove 'continued on the next line' backslash if present. */
				if ('\\' == (*riter2)) {
					riter1++;
					retval.erase(riter2.base()--);
					break;
				} else if (!std::isspace(*riter2)) {
					break;
				}

				riter2++;
			}
		}

		riter1++;
	}

	return retval;
}

static std::vector<std::string> f_declared_object_strings(const std::string& decl_stmt_str) {
	std::vector<std::string> retval;

	auto nice_decl_stmt_str = with_newlines_removed(decl_stmt_str);
	auto semicolon_position = std::string::npos;
	for (size_t pos = 3; pos < nice_decl_stmt_str.size(); pos += 1) {
		if (';' == nice_decl_stmt_str[pos]) {
			semicolon_position = pos;
		}
	}
	if (std::string::npos == semicolon_position) {
		assert(false);
		return retval;
	}

	std::vector<size_t> delimiter_positions;
	for (size_t pos = 3; ((pos < nice_decl_stmt_str.size()) && (pos < semicolon_position)); pos += 1) {
		if (',' == nice_decl_stmt_str[pos]) {
			delimiter_positions.push_back(pos);
		}
	}

	delimiter_positions.push_back(semicolon_position);
	auto first_delimiter_pos = delimiter_positions[0];

	{
		auto pos1 = first_delimiter_pos - 1;
		auto pos2 = pos1;
		bool nonspace_found = false;
		while ((2 <= pos1) && (!nonspace_found)) {
			if (!std::isspace(nice_decl_stmt_str[pos1])) {
				pos2 = pos1 + 1;
				nonspace_found = true;
			}

			pos1 -= 1;
		}
		if (!nonspace_found) {
			assert(false);
			return retval;
		}

		bool space_found = false;
		while ((1 <= pos1) && (!space_found)) {
			if (std::isspace(nice_decl_stmt_str[pos1])) {
				space_found = true;
			}

			pos1 -= 1;
		}
		if (!space_found) {
			assert(false);
			return retval;
		}

		pos1 += 2;
		std::string first_declaration_string = nice_decl_stmt_str.substr(pos1, pos2 - pos1);
		retval.push_back(first_declaration_string);
	}

	{
		size_t delimiter_index = 0;
		while (delimiter_positions.size() > (delimiter_index + 1)) {
			if (!(delimiter_positions[delimiter_index] + 1 < delimiter_positions[(delimiter_index + 1)])) {
				//assert(false);
			} else {
				std::string declaration_string = nice_decl_stmt_str.substr(delimiter_positions[delimiter_index] + 1, delimiter_positions[(delimiter_index + 1)] - (delimiter_positions[delimiter_index] + 1));
				retval.push_back(declaration_string);
			}

			delimiter_index += 1;
		}
	}

	return retval;
}

class CState1;

class CDeclReplacementActionRecord {
public:
	CDeclReplacementActionRecord(Rewriter &Rewrite, const clang::DeclaratorDecl& ddecl, const std::string& replacement_text
			, const std::string& action_species)
: m_Rewrite_ptr(&Rewrite), m_ddecl_cptr(&ddecl), m_replacement_text(replacement_text), m_action_species(action_species) {
	}
	virtual ~CDeclReplacementActionRecord() {}
	clang::SourceRange source_range() {
		clang::SourceRange retval = m_ddecl_cptr->getSourceRange();
		return retval;
	}
	clang::SourceLocation start_location() {
		clang::SourceLocation retval = source_range().getBegin();
		return retval;
	}
	std::string get_var_name() {
		std::string retval = m_ddecl_cptr->getNameAsString();
		return retval;
	}
	const clang::DeclaratorDecl* get_ddecl_cptr() { return m_ddecl_cptr; }
	std::string replacement_text() { return m_replacement_text; }
	std::string action_species() { return m_action_species; }

	Rewriter* m_Rewrite_ptr = nullptr;
	const clang::DeclaratorDecl* m_ddecl_cptr = nullptr;
	std::string m_replacement_text;
	std::string m_action_species;
};

class CDeclReplacementActionRecordsLog : public std::vector<CDeclReplacementActionRecord> {
public:
	CDeclReplacementActionRecordsLog::iterator find(const clang::DeclaratorDecl* ddecl_cptr) {
		CDeclReplacementActionRecordsLog::iterator retval = (*this).end();
		CDeclReplacementActionRecordsLog::iterator iter = (*this).begin();
		for (CDeclReplacementActionRecordsLog::iterator iter = (*this).begin(); (*this).end() != iter; iter++) {
			if ((*iter).get_ddecl_cptr() == ddecl_cptr) {
				retval = iter;
				break;
			}
		}
		return retval;
	}
	CDeclReplacementActionRecordsLog::iterator find(const clang::SourceLocation& start_location) {
		CDeclReplacementActionRecordsLog::iterator retval = (*this).end();
		for (CDeclReplacementActionRecordsLog::iterator iter = (*this).begin(); (*this).end() != iter; iter++) {
			auto l_sl = (*iter).start_location();
			if ((start_location.isValid()) && (l_sl.isValid()) && (start_location == l_sl)) {
				retval = iter;
				break;
			}
		}
		return retval;
	}
};

class CReplacementAction {
public:
	virtual ~CReplacementAction() {}
	virtual void do_replacement(CState1& state1) const = 0;
};

class CDDeclReplacementAction : public CReplacementAction {
public:
	CDDeclReplacementAction(Rewriter &Rewrite, const MatchFinder::MatchResult &MR,
			const clang::DeclaratorDecl& ddecl) : m_Rewrite(Rewrite), m_MR(MR), m_ddecl_cptr(&ddecl) {
	}
	virtual ~CDDeclReplacementAction() {}

	virtual void do_replacement(CState1& state1) const = 0;
	virtual const clang::DeclaratorDecl* get_ddecl_cptr() const { return m_ddecl_cptr; }

	clang::SourceRange source_range() {
		clang::SourceRange retval = m_ddecl_cptr->getSourceRange();
		return retval;
	}
	clang::SourceLocation start_location() {
		clang::SourceLocation retval = source_range().getBegin();
		return retval;
	}
	std::string get_var_name() {
		std::string retval = m_ddecl_cptr->getNameAsString();
		return retval;
	}

	Rewriter& m_Rewrite;
	const MatchFinder::MatchResult m_MR;
	const clang::DeclaratorDecl* m_ddecl_cptr = nullptr;
};

class CArrayReplacementAction : public CDDeclReplacementAction {
public:
	using CDDeclReplacementAction::CDDeclReplacementAction;
	virtual ~CArrayReplacementAction() {}
};

class CDynamicArrayReplacementAction : public CArrayReplacementAction {
public:
	using CArrayReplacementAction::CArrayReplacementAction;
	virtual ~CDynamicArrayReplacementAction() {}
};

class CSetArrayPointerToNullReplacementAction : public CDynamicArrayReplacementAction {
public:
	CSetArrayPointerToNullReplacementAction(Rewriter &Rewrite, const MatchFinder::MatchResult &MR, const clang::DeclaratorDecl& ddecl,
			const BinaryOperator* BO, const DeclRefExpr* DRE, const MemberExpr* ME)
: CDynamicArrayReplacementAction(Rewrite, MR, ddecl), m_BO(BO), m_DRE(DRE), m_ME(ME) {
	}
	virtual ~CSetArrayPointerToNullReplacementAction() {}

	virtual void do_replacement(CState1& state1) const {
		Rewriter &Rewrite = m_Rewrite;
		const MatchFinder::MatchResult &MR = m_MR;
		const BinaryOperator* BO = m_BO;
		const Expr* RHS = nullptr;
		const Expr* LHS = nullptr;
		if (BO != nullptr) {
			RHS = BO->getRHS();
			LHS = BO->getLHS();
		}
		const DeclRefExpr* DRE = m_DRE;
		const MemberExpr* ME = m_ME;

		if ((BO != nullptr) && (RHS != nullptr) && (LHS != nullptr) && (DRE != nullptr))
		{
			auto BOSR = nice_source_range(BO->getSourceRange(), Rewrite);
			SourceLocation BOSL = BOSR.getBegin();
			SourceLocation BOSLE = BOSR.getEnd();

			ASTContext *const ASTC = MR.Context;
			FullSourceLoc FBOSL = ASTC->getFullLoc(BOSL);

			SourceManager &SM = ASTC->getSourceManager();

			auto source_location_str = BOSL.printToString(*MR.SourceManager);
			std::string source_text;
			if (BOSL.isValid() && BOSLE.isValid()) {
				source_text = Rewrite.getRewrittenText(SourceRange(BOSL, BOSLE));
			} else {
				return;
			}

			if (filtered_out_by_location(MR, BOSL)) {
				return void();
			}

			Expr::NullPointerConstantKind kind = RHS->isNullPointerConstant(*ASTC, Expr::NullPointerConstantValueDependence());
			if (clang::Expr::NPCK_NotNull != kind) {
				auto lhs_source_range = nice_source_range(LHS->getSourceRange(), Rewrite);
				std::string lhs_source_text;
				if (lhs_source_range.isValid()) {
					lhs_source_text = Rewrite.getRewrittenText(lhs_source_range);
					//auto lhs_source_text_sans_ws = with_whitespace_removed(lhs_source_text);

					QualType QT;
					std::string element_type_str;
					clang::SourceRange decl_source_range;
					std::string variable_name;
					std::string bo_replacement_code;
					const clang::DeclaratorDecl* DD = nullptr;

					auto decl = DRE->getDecl();
					auto VD = dynamic_cast<const VarDecl*>(decl);

					const clang::FieldDecl* FD = nullptr;
					if (nullptr != ME) {
						auto member_decl = ME->getMemberDecl();
						FD = dynamic_cast<const clang::FieldDecl*>(ME->getMemberDecl());
					}
					if (nullptr != FD) {
						DD = FD;

						auto field_decl_source_range = nice_source_range(FD->getSourceRange(), Rewrite);
						auto field_decl_source_location_str = field_decl_source_range.getBegin().printToString(*MR.SourceManager);
						std::string field_decl_source_text;
						if (field_decl_source_range.isValid()) {
							field_decl_source_text = Rewrite.getRewrittenText(field_decl_source_range);
							decl_source_range = field_decl_source_range;
						} else {
							return;
						}
						QT = FD->getType();
						variable_name = FD->getNameAsString();
					} else if (nullptr != VD) {
						DD = VD;
						auto decl_source_range = nice_source_range(VD->getSourceRange(), Rewrite);

						auto qualified_name = VD->getQualifiedNameAsString();
						static const std::string mse_namespace_str1 = "mse::";
						static const std::string mse_namespace_str2 = "::mse::";
						if ((0 == qualified_name.compare(0, mse_namespace_str1.size(), mse_namespace_str1))
								|| (0 == qualified_name.compare(0, mse_namespace_str2.size(), mse_namespace_str2))) {
							return;
						}

						QT = VD->getType();
						variable_name = VD->getNameAsString();
					} else {
						int q = 7;
					}

					if (nullptr != DD) {
						if (true) {
							if (true) {
								bo_replacement_code = lhs_source_text;
								bo_replacement_code += ".resize(0)";

								auto BOSR = clang::SourceRange(BOSL, BOSLE);
								if (ConvertToSCPP && (BOSR.isValid())) {
									auto res2 = Rewrite.ReplaceText(BOSR, bo_replacement_code);
									int q = 3;
								} else {
									int q = 7;
								}
							} else {
								int q = 5;
							}
						}
					}
					int q = 5;
				} else {
					int q = 5;
				}
				int q = 5;
			}
		}
	}

	const BinaryOperator* m_BO;
	const DeclRefExpr* m_DRE;
	const MemberExpr* m_ME;
};

std::string tolowerstr(const std::string& a) {
	std::string retval;
	for (const auto& ch : a) {
		retval += tolower(ch);
	}
	return retval;
}

class CFreeDynamicArrayReplacementAction : public CDynamicArrayReplacementAction {
public:
	CFreeDynamicArrayReplacementAction(Rewriter &Rewrite, const MatchFinder::MatchResult &MR, const clang::DeclaratorDecl& ddecl,
			const CallExpr* CE, const DeclRefExpr* DRE, const MemberExpr* ME)
: CDynamicArrayReplacementAction(Rewrite, MR, ddecl), m_CE(CE), m_DRE(DRE), m_ME(ME) {
	}
	virtual ~CFreeDynamicArrayReplacementAction() {}

	virtual void do_replacement(CState1& state1) const {
		Rewriter &Rewrite = m_Rewrite;
		const MatchFinder::MatchResult &MR = m_MR;
		const CallExpr* CE = m_CE;
		const DeclRefExpr* DRE = m_DRE;
		const MemberExpr* ME = m_ME;

		if ((CE != nullptr) && (DRE != nullptr))
		{
			auto CESR = nice_source_range(CE->getSourceRange(), Rewrite);
			SourceLocation CESL = CESR.getBegin();
			SourceLocation CESLE = CESR.getEnd();

			ASTContext *const ASTC = MR.Context;
			FullSourceLoc FCESL = ASTC->getFullLoc(CESL);

			SourceManager &SM = ASTC->getSourceManager();

			auto source_location_str = CESL.printToString(*MR.SourceManager);
			std::string source_text;
			if (CESL.isValid() && CESLE.isValid()) {
				source_text = Rewrite.getRewrittenText(SourceRange(CESL, CESLE));
			} else {
				return;
			}

			if (filtered_out_by_location(MR, CESL)) {
				return void();
			}

			auto function_decl = CE->getDirectCallee();
			auto num_args = CE->getNumArgs();
			if (function_decl && (1 == num_args)) {
				{
					std::string function_name = function_decl->getNameAsString();
					static const std::string free_str = "free";
					auto lc_function_name = tolowerstr(function_name);
					bool ends_with_free = ((lc_function_name.size() >= free_str.size())
							&& (0 == lc_function_name.compare(lc_function_name.size() - free_str.size(), free_str.size(), free_str)));
					if (ends_with_free) {
						auto iter = CE->arg_begin();
						assert((*iter)->getType().getTypePtrOrNull());
						auto arg_source_range = nice_source_range((*iter)->getSourceRange(), Rewrite);
						std::string arg_source_text;
						if (arg_source_range.isValid()) {
							arg_source_text = Rewrite.getRewrittenText(arg_source_range);
							//auto arg_source_text_sans_ws = with_whitespace_removed(arg_source_text);
							QualType QT;
							std::string element_type_str;
							clang::SourceRange decl_source_range;
							std::string variable_name;
							std::string ce_replacement_code;
							const clang::DeclaratorDecl* DD = nullptr;

							auto decl = DRE->getDecl();
							DD = dynamic_cast<const DeclaratorDecl*>(decl);
							auto VD = dynamic_cast<const VarDecl*>(decl);

							const clang::FieldDecl* FD = nullptr;
							if (nullptr != ME) {
								auto member_decl = ME->getMemberDecl();
								FD = dynamic_cast<const clang::FieldDecl*>(ME->getMemberDecl());
							}
							if (nullptr != FD) {
								DD = FD;
							} else if (nullptr != VD) {
								DD = VD;
							} else {
								int q = 7;
							}

							if (nullptr != DD) {
								auto decl_source_range = nice_source_range(DD->getSourceRange(), Rewrite);
								auto decl_source_location_str = decl_source_range.getBegin().printToString(*MR.SourceManager);
								std::string decl_source_text;
								if (decl_source_range.isValid()) {
									decl_source_text = Rewrite.getRewrittenText(decl_source_range);
								} else {
									return;
								}
								QT = DD->getType();
								variable_name = DD->getNameAsString();

								auto qualified_name = DD->getQualifiedNameAsString();
								static const std::string mse_namespace_str1 = "mse::";
								static const std::string mse_namespace_str2 = "::mse::";
								if ((0 == qualified_name.compare(0, mse_namespace_str1.size(), mse_namespace_str1))
										|| (0 == qualified_name.compare(0, mse_namespace_str2.size(), mse_namespace_str2))) {
									return;
								}

								if (true) {
									ce_replacement_code = arg_source_text;
									ce_replacement_code += ".resize(0)";

									auto CESR = clang::SourceRange(CESL, CESLE);
									if (ConvertToSCPP && (CESR.isValid())) {
										auto res2 = Rewrite.ReplaceText(CESR, ce_replacement_code);
										int q = 3;
									} else {
										int q = 7;
									}
								}
							}
							int q = 5;
						} else {
							int q = 5;
						}
						int q = 5;
					}
				}

			}
		}
	}

	const CallExpr* m_CE;
	const DeclRefExpr* m_DRE;
	const MemberExpr* m_ME;
};

static std::vector<const DeclaratorDecl*> IndividualDeclaratorDecls(const DeclaratorDecl* VD, Rewriter &Rewrite) {
	std::vector<const DeclaratorDecl*> retval;

	if (!VD) {
		assert(false);
		return retval;
	}
	auto SR = nice_source_range(VD->getSourceRange(), Rewrite);
	SourceLocation SL = SR.getBegin();

	auto decl_context = VD->getDeclContext();
	if ((!decl_context) || (!SL.isValid())) {
		assert(false);
		retval.push_back(VD);
	} else {
		for (auto decl_iter = decl_context->decls_begin(); decl_iter != decl_context->decls_end(); decl_iter++) {
			auto decl = (*decl_iter);
			auto var_decl = dynamic_cast<const DeclaratorDecl*>(decl);
			if (var_decl) {
				auto VDSR = nice_source_range(var_decl->getSourceRange(), Rewrite);
				SourceLocation l_SL = VDSR.getBegin();
				if (l_SL == SL) {
					retval.push_back(var_decl);
				}
			}
		}
	}
	if (0 == retval.size()) {
		assert(false);
	}

	return retval;
}

class CMallocArrayReplacementAction : public CArrayReplacementAction {
public:
	CMallocArrayReplacementAction(Rewriter &Rewrite, const MatchFinder::MatchResult &MR, const clang::DeclaratorDecl& ddecl,
			const BinaryOperator* BO, const std::string& bo_replacement_code, const std::string& declaration_replacement_code)
: CArrayReplacementAction(Rewrite, MR, ddecl), m_BO(BO), m_DD(&ddecl), m_bo_replacement_code(bo_replacement_code), m_declaration_replacement_code(declaration_replacement_code) {
	}
	virtual ~CMallocArrayReplacementAction() {}

	virtual void do_replacement(CState1& state1) const;

	const BinaryOperator* m_BO = nullptr;
	const CallExpr* m_CE = nullptr;
	const DeclRefExpr* m_DRE = nullptr;
	const MemberExpr* m_ME = nullptr;
	const DeclaratorDecl* m_DD = nullptr;
	std::string m_bo_replacement_code;
	std::string m_declaration_replacement_code;
};

class CDDeclReplacementActionMap : public std::multimap<const clang::DeclaratorDecl*, std::shared_ptr<CDDeclReplacementAction>> {
public:
	typedef std::multimap<const clang::DeclaratorDecl*, std::shared_ptr<CDDeclReplacementAction>> base_class;
	iterator insert( const std::shared_ptr<CDDeclReplacementAction>& cr_shptr ) {
		iterator retval(end());
		if (!cr_shptr) { assert(false); } else {
			value_type val((*cr_shptr).get_ddecl_cptr(), cr_shptr);
			retval = base_class::insert(val);
		}
		return retval;
	}
	void do_and_dispose_matching_replacements(CState1& state1, const clang::DeclaratorDecl& ddecl) {
		auto DD = &ddecl;
		auto range = base_class::equal_range(DD);
		while (range.first != range.second) {
			for (auto iter = range.first; range.second != iter; iter++) {
				(*((*iter).second)).do_replacement(state1);
			}
			base_class::erase(range.first, range.second);
			range = base_class::equal_range(DD);
		}
	}
};

class CDynamicArrayReplacementActionMap : public CDDeclReplacementActionMap {
public:
	iterator insert( const std::shared_ptr<CDynamicArrayReplacementAction>& cr_shptr ) {
		return CDDeclReplacementActionMap::insert(static_cast<std::shared_ptr<CDDeclReplacementAction> >(cr_shptr));
	}
};

class CArrayReplacementActionMap : public CDDeclReplacementActionMap {
public:
	iterator insert( const std::shared_ptr<CArrayReplacementAction>& cr_shptr ) {
		return CDDeclReplacementActionMap::insert(static_cast<std::shared_ptr<CDDeclReplacementAction> >(cr_shptr));
	}
};

class CState1 {
public:
	CDeclReplacementActionRecordsLog m_decl_replacement_action_records_log;
	CDynamicArrayReplacementActionMap m_dynamic_array_contingent_replacement_map;
	CArrayReplacementActionMap m_array_contingent_replacement_map;
};

void CMallocArrayReplacementAction::do_replacement(CState1& state1) const {
	Rewriter &Rewrite = m_Rewrite;
	const MatchFinder::MatchResult &MR = m_MR;
	const BinaryOperator* BO = m_BO;
	const DeclaratorDecl* DD = m_DD;

	if ((BO != nullptr) && (DD != nullptr))
	{
		auto BOSR = nice_source_range(BO->getSourceRange(), Rewrite);
		auto decl_source_range = nice_source_range(DD->getSourceRange(), Rewrite);
		auto decl_source_location_str = decl_source_range.getBegin().printToString(*MR.SourceManager);
		std::string decl_source_text;
		if (decl_source_range.isValid()) {
			decl_source_text = Rewrite.getRewrittenText(decl_source_range);
		} else {
			return;
		}

		auto decls = IndividualDeclaratorDecls(DD, Rewrite);

		if (ConvertToSCPP && decl_source_range.isValid() && (BOSR.isValid())) {
			if ((3 <= m_declaration_replacement_code.size())) {
				for (const auto& decl : decls) {
					auto iter = state1.m_decl_replacement_action_records_log.find(decl);
					if (state1.m_decl_replacement_action_records_log.end() != iter) {
						/* This declaration had already been replaced. We're just going to
						 * overwrite it. */
						 state1.m_decl_replacement_action_records_log.erase(iter);
					}

					CDeclReplacementActionRecord action_record(Rewrite, *decl, m_declaration_replacement_code, "pointer targeting heap allocated array to mse vector iterator");
					state1.m_decl_replacement_action_records_log.push_back(action_record);
					state1.m_dynamic_array_contingent_replacement_map.do_and_dispose_matching_replacements(state1, *decl);
				}
				auto res = Rewrite.ReplaceText(decl_source_range, m_declaration_replacement_code);
			}
			auto res2 = Rewrite.ReplaceText(BOSR, m_bo_replacement_code);
			int q = 3;
		} else {
			int q = 7;
		}
	}
}

/**********************************************************************************************************************/
class MCSSSArrayToPointerDecay : public MatchFinder::MatchCallback
{
public:
	MCSSSArrayToPointerDecay (Rewriter &Rewrite, CState1& state1)
: Rewrite(Rewrite), m_state1(state1) {}

	virtual void run(const MatchFinder::MatchResult &MR)
	{
		if (/*SafeSubset && */(MR.Nodes.getNodeAs<clang::CastExpr>("mcsssarraytopointerdecay") != nullptr))
		{
			const CastExpr* CE = MR.Nodes.getNodeAs<clang::CastExpr>("mcsssarraytopointerdecay");

			auto SR = nice_source_range(CE->getSourceRange(), Rewrite);
			SourceLocation SL = SR.getBegin();
			SourceLocation SLE = SR.getEnd();

			ASTContext *const ASTC = MR.Context;
			FullSourceLoc FSL = ASTC->getFullLoc(SL);

			auto source_location_str = SL.printToString(*MR.SourceManager);
			std::string source_text;
			if (SL.isValid() && SLE.isValid()) {
				source_text = Rewrite.getRewrittenText(SourceRange(SL, SLE));
			} else {
				return;
			}

			if (filtered_out_by_location(MR, SL))
			{
				/*intentionally left blank*/
			}
			else
			{
				{
					if (false) {
						std::cout << "sss1.2:" << "array to pointer decay:";
						std::cout << SL.printToString(*MR.SourceManager) << ":" << std::endl;

						//XMLDocOut.XMLAddNode(MR.Context, SL, "sss1.2", "array to pointer decay: ");
						//JSONDocOUT.JSONAddElement(MR.Context, SL, "sss1.2", "array to pointer decay: ");
					}
				}
			}
		}
	}

private:
	Rewriter &Rewrite;
	CState1& m_state1;
};
/**********************************************************************************************************************/
class MCSSSNativePointer : public MatchFinder::MatchCallback
{
public:
	MCSSSNativePointer (Rewriter &Rewrite) : Rewrite(Rewrite) {}

	virtual void run(const MatchFinder::MatchResult &MR)
	{
		if (SafeSubset && (MR.Nodes.getNodeAs<clang::VarDecl>("mcsssnativepointer") != nullptr))
		{
			const VarDecl *VD = MR.Nodes.getNodeAs<clang::VarDecl>("mcsssnativepointer");

			auto SR = nice_source_range(VD->getSourceRange(), Rewrite);
			SourceLocation SL = SR.getBegin();
			SourceLocation SLE = SR.getEnd();

			ASTContext* const ASTC = MR.Context;
			FullSourceLoc FSL = ASTC->getFullLoc(SL);

			auto source_location_str = SL.printToString(*MR.SourceManager);
			std::string source_text;
			if (SL.isValid() && SLE.isValid()) {
				source_text = Rewrite.getRewrittenText(SourceRange(SL, SLE));
			} else {
				return;
			}

			if (filtered_out_by_location(MR, SL))
			{
				/*intentionally left blank*/
			}
			else
			{
				{
					if (false && SafeSubset) {
						std::cout << "sss1.1:" << "native pointer:";
						std::cout << SL.printToString(*MR.SourceManager) << ":" << std::endl;

						//XMLDocOut.XMLAddNode(MR.Context, SL, "sss1.1", "native pointer: ");
						//JSONDocOUT.JSONAddElement(MR.Context, SL, "sss1.1", "native pointer: ");
					}
				}
			}
		}
	}

	virtual void onEndOfTranslationUnit()
	{
	}

private:
	Rewriter &Rewrite;
};
/**********************************************************************************************************************/
class MCSSSVarDecl : public MatchFinder::MatchCallback
{
public:
	MCSSSVarDecl (Rewriter &Rewrite, CState1& state1)
: Rewrite(Rewrite), m_state1(state1) {}

	virtual void run(const MatchFinder::MatchResult &MR)
	{
		if ((MR.Nodes.getNodeAs<clang::VarDecl>("mcsssvardecl") != nullptr))
		{
			const VarDecl* VD = MR.Nodes.getNodeAs<clang::VarDecl>("mcsssvardecl");

			auto SR = nice_source_range(VD->getSourceRange(), Rewrite);
			SourceLocation SL = SR.getBegin();
			SourceLocation SLE = SR.getEnd();

			QualType QT = VD->getType();

			const clang::Type* TP = QT.getTypePtr();

			ASTContext *const ASTC = MR.Context;
			FullSourceLoc FSL = ASTC->getFullLoc(SL);

			SourceManager &SM = ASTC->getSourceManager();

			auto source_location_str = SL.printToString(*MR.SourceManager);
			std::string source_text;
			if (SL.isValid() && SLE.isValid()) {
				source_text = Rewrite.getRewrittenText(SourceRange(SL, SLE));
			} else {
				return;
			}

			if (filtered_out_by_location(MR, SL)) {
				return void();
			}

			auto storage_duration = VD->getStorageDuration();
			bool has_dynamic_storage_duration = (clang::StorageDuration::SD_Dynamic == storage_duration);
			bool is_a_temporary = (clang::StorageDuration::SD_FullExpression == storage_duration);
			bool is_static = (clang::StorageDuration::SD_Static == storage_duration);
			bool is_a_function_parameter = (VD->isLocalVarDeclOrParm() && (!VD->isLocalVarDecl()));

			auto variable_name = VD->getName();
			auto variable_name2 = VD->getNameAsString();
			std::string identifier_name_str;
			auto pIdentifier = VD->getIdentifier();
			if (pIdentifier) {
				identifier_name_str = pIdentifier->getName();
			}

			std::string initialization_expr_str;
			auto pInitExpr = VD->getInit();
			if (VD->hasInit() && pInitExpr) {
				auto init_expr_source_range = nice_source_range(pInitExpr->getSourceRange(), Rewrite);
				initialization_expr_str = Rewrite.getRewrittenText(init_expr_source_range);
			}

			if (TP->isArrayType()) {
				auto ATP = static_cast<const clang::ArrayType*>(TP);
				auto element_type = ATP->getElementType();
				auto elementSplitQualType = element_type.split();
				auto element_type_str = clang::QualType::getAsString(elementSplitQualType);

				std::string replacement_code;
				if (is_static) {
					replacement_code += "static ";
				}
				replacement_code += "mse::mstd::array<";
				replacement_code += element_type_str;
				replacement_code += ", ";

				if (TP->isConstantArrayType()) {
					auto CATP = static_cast<const clang::ConstantArrayType*>(TP);
					if (!CATP) {
						assert(false);
					} else {
						auto array_size = CATP->getSize();

						auto left_bracket_pos = source_text.find('[');
						auto right_bracket_pos = source_text.find(']');
						if ((std::string::npos != left_bracket_pos) && (std::string::npos != right_bracket_pos)
								&& (left_bracket_pos + 1 < right_bracket_pos)) {
							auto array_size_expression_text = source_text.substr(left_bracket_pos + 1, right_bracket_pos - (left_bracket_pos + 1));

							replacement_code += array_size_expression_text;
							int q = 3;
						} else {
							int q = 7;
						}
						int q = 5;
					}
				} else if (TP->isVariableArrayType()) {
					auto VATP = static_cast<const clang::VariableArrayType*>(TP);
					if (!VATP) {
						assert(false);
					} else {
						auto size_expr = VATP->getSizeExpr();
					}
				}

				replacement_code += "> ";
				std::string new_array_variable_name = variable_name;
				new_array_variable_name += "_array";
				replacement_code += new_array_variable_name;
				if ("" != initialization_expr_str) {
					replacement_code += " = ";
					replacement_code += initialization_expr_str;
				}
				replacement_code += "; \n";
				if (is_static) {
					replacement_code += "static ";
				}
				replacement_code += "auto ";
				replacement_code += variable_name;
				replacement_code += " = ";
				replacement_code += new_array_variable_name;
				replacement_code += ".begin()";
				if (ConvertToSCPP && SL.isValid() && SLE.isValid()) {
					auto res = Rewrite.ReplaceText(SourceRange(SL, SLE), replacement_code);
					CDeclReplacementActionRecord action_record(Rewrite, *VD, replacement_code, "native to mse array");
					m_state1.m_decl_replacement_action_records_log.push_back(action_record);
					m_state1.m_dynamic_array_contingent_replacement_map.do_and_dispose_matching_replacements(m_state1, *VD);
					int q = 3;
				} else {
					int q = 7;
				}
				m_state1.m_dynamic_array_contingent_replacement_map.do_and_dispose_matching_replacements(m_state1, *VD);
				int q = 5;
			} else {
				;
			}

		}
	}

	virtual void onEndOfTranslationUnit()
	{
	}

private:
	Rewriter &Rewrite;
	CState1& m_state1;
};

/**********************************************************************************************************************/
class CRandomAccessIteratorFromPointerDDeclRetval {
public:
	std::string m_replacement_code;
	std::string m_action_species;
};
static CRandomAccessIteratorFromPointerDDeclRetval RandomAccessIteratorFromPointerDDecl(const DeclaratorDecl* DD, Rewriter &Rewrite) {
	CRandomAccessIteratorFromPointerDDeclRetval retval;

	QualType QT = DD->getType();

	const clang::Type* TP = QT.getTypePtr();

	clang::StorageDuration storage_duration = clang::StorageDuration::SD_Automatic;
	bool has_dynamic_storage_duration = false;
	bool is_a_temporary = false;
	bool is_static = false;
	bool is_a_function_parameter = false;
	std::string initialization_expr_str;

	auto VD = dynamic_cast<const clang::VarDecl *>(DD);
	if (VD) {
		storage_duration = VD->getStorageDuration();
		has_dynamic_storage_duration = (clang::StorageDuration::SD_Dynamic == storage_duration);
		is_a_temporary = (clang::StorageDuration::SD_FullExpression == storage_duration);
		is_static = (clang::StorageDuration::SD_Static == storage_duration);
		is_a_function_parameter = (VD->isLocalVarDeclOrParm() && (!VD->isLocalVarDecl()));

		auto pInitExpr = VD->getInit();
		if (VD->hasInit() && pInitExpr) {
			auto init_expr_source_range = nice_source_range(pInitExpr->getSourceRange(), Rewrite);
			initialization_expr_str = Rewrite.getRewrittenText(init_expr_source_range);
		}
	}

	auto variable_name = DD->getNameAsString();
	std::string identifier_name_str;
	auto pIdentifier = DD->getIdentifier();
	if (pIdentifier) {
		identifier_name_str = pIdentifier->getName();
	}

	bool replacement_code_generated = false;
	if (TP->isPointerType()) {
		auto TPP = static_cast<const clang::PointerType*>(TP);
		if (TPP) {
			auto target_type = TPP->getPointeeType();

			auto splitQualType = target_type.split();
			auto type_str = clang::QualType::getAsString(splitQualType);

			if (("char" != type_str) && ("const char" != type_str)) {
				std::string replacement_code;
				if (is_static) {
					replacement_code += "static ";
				}
				replacement_code += "mse::TNullableAnyRandomAccessIterator<";
				replacement_code += type_str;
				replacement_code += "> ";
				replacement_code += variable_name;

				if ("" != initialization_expr_str) {
					replacement_code += " = ";
					replacement_code += initialization_expr_str;
				}
				retval.m_replacement_code = replacement_code;
				retval.m_action_species = "pointer to random access iterator";
				replacement_code_generated = true;
			} else {
				int q = 3;
			}
		} else {
			assert(false);
			int q = 1;
		}
	}

	if (!replacement_code_generated) {
		auto splitQualType = QT.split();
		auto type_str = clang::QualType::getAsString(splitQualType);

		std::string replacement_code;
		if (is_static) {
			replacement_code += "static ";
		}
		replacement_code += type_str;
		replacement_code += " ";
		replacement_code += variable_name;

		if ("" != initialization_expr_str) {
			replacement_code += " = ";
			replacement_code += initialization_expr_str;
		}
		retval.m_replacement_code = replacement_code;
		retval.m_action_species = "char*";
		replacement_code_generated = true;
	}
	return retval;
}

class MCSSSPointerArithmetic : public MatchFinder::MatchCallback
{
public:
	MCSSSPointerArithmetic (Rewriter &Rewrite, CState1& state1)
: Rewrite(Rewrite), m_state1(state1) {}

	virtual void run(const MatchFinder::MatchResult &MR)
	{
		const DeclRefExpr* DRE = MR.Nodes.getNodeAs<clang::DeclRefExpr>("mcssspointerarithmetic");
		const MemberExpr* ME = MR.Nodes.getNodeAs<clang::MemberExpr>("mcssspointerarithmetic2");

		if (DRE != nullptr)
		{
			const DeclRefExpr* DRE = MR.Nodes.getNodeAs<clang::DeclRefExpr>("mcssspointerarithmetic");

			auto SR = nice_source_range(DRE->getSourceRange(), Rewrite);
			SourceLocation SL = SR.getBegin();
			SourceLocation SLE = SR.getEnd();

			QualType QT = DRE->getType();

			const clang::Type* TP = QT.getTypePtr();

			ASTContext *const ASTC = MR.Context;
			FullSourceLoc FSL = ASTC->getFullLoc(SL);

			SourceManager &SM = ASTC->getSourceManager();

			auto source_location_str = SL.printToString(*MR.SourceManager);
			std::string source_text;
			if (SL.isValid() && SLE.isValid()) {
				source_text = Rewrite.getRewrittenText(SourceRange(SL, SLE));
			} else {
				return;
			}


			if (filtered_out_by_location(MR, SL)) {
				return void();
			}

			auto decl = DRE->getDecl();
			auto DD = dynamic_cast<const DeclaratorDecl*>(decl);

			const clang::FieldDecl* FD = nullptr;
			if (nullptr != ME) {
				auto member_decl = ME->getMemberDecl();
				FD = dynamic_cast<const clang::FieldDecl*>(ME->getMemberDecl());
			}
			if (nullptr != FD) {
				DD = FD;
			}

			if (!DD) {
				return;
			} else {
				auto decl_source_range = nice_source_range(DD->getSourceRange(), Rewrite);
				auto decl_source_location_str = decl_source_range.getBegin().printToString(*MR.SourceManager);
				std::string decl_source_text;
				if (decl_source_range.isValid()) {
					decl_source_text = Rewrite.getRewrittenText(decl_source_range);
				} else {
					return;
				}
				QT = DD->getType();
				std::string variable_name = DD->getNameAsString();

				auto qualified_name = DD->getQualifiedNameAsString();
				static const std::string mse_namespace_str1 = "mse::";
				static const std::string mse_namespace_str2 = "::mse::";
				if ((0 == qualified_name.compare(0, mse_namespace_str1.size(), mse_namespace_str1))
						|| (0 == qualified_name.compare(0, mse_namespace_str2.size(), mse_namespace_str2))) {
					return;
				}

				auto iter = m_state1.m_decl_replacement_action_records_log.find(DD);
				if (m_state1.m_decl_replacement_action_records_log.end() != iter) {
					/* This variable declaration has already been processed. Probably. */
					return;
				}

				auto var_decls = IndividualDeclaratorDecls(DD, Rewrite);
				if ((1 <= var_decls.size()) && (var_decls.back() == DD)) {
					std::vector<std::string> action_species_list;
					std::string replacement_code;
					for (const auto& var_decl : var_decls) {
						auto res = RandomAccessIteratorFromPointerDDecl(var_decl, Rewrite);
						action_species_list.push_back(res.m_action_species);
						replacement_code += res.m_replacement_code;
						replacement_code += "; \n";
					}
					if (replacement_code.size() >= 3) {
						replacement_code = replacement_code.substr(0, replacement_code.size() - 3);
					}

					if (ConvertToSCPP && decl_source_range.isValid() && (3 <= replacement_code.size())) {
						auto res = Rewrite.ReplaceText(decl_source_range, replacement_code);

						for (auto var_decl : var_decls) {
							assert(1 <= action_species_list.size());
							CDeclReplacementActionRecord action_record(Rewrite, *var_decl, replacement_code, action_species_list.front());
							action_species_list.erase(action_species_list.begin());
							m_state1.m_decl_replacement_action_records_log.push_back(action_record);
							m_state1.m_array_contingent_replacement_map.do_and_dispose_matching_replacements(m_state1, *var_decl);
						}
						int q = 3;
					} else {
						int q = 7;
					}
				} else {
					int q = 7;
				}
			}
		}
	}

private:
	Rewriter &Rewrite;
	CState1& m_state1;
};

/**********************************************************************************************************************/

class CIndirectionState {
public:
	CIndirectionState(std::string original, std::string current)
		: m_original(original), m_current(current) {}
	CIndirectionState(const CIndirectionState& src) = default;
	std::string m_original;
	std::string m_current;
};

std::vector<CIndirectionState>& populateTypeIndirectionStack(std::vector<CIndirectionState>& stack, const clang::Type& type, int depth = 0) {
	auto TP = &type;
	if (TP->isArrayType()) {
		const clang::ArrayType* ATP = TP->getAsArrayTypeUnsafe();
		if (ATP) {
			auto QT = ATP->getElementType();
			auto l_TP = QT.getTypePtr();
			auto type_str = clang::QualType::getAsString(QT.split());

			stack.push_back(CIndirectionState("native array", "native array"));

			return populateTypeIndirectionStack(stack, *l_TP, depth+1);
		} else {
			assert(false);
		}
	} else if (TP->isPointerType()) {
		auto PTP = static_cast<const clang::PointerType*>(TP);
		if (PTP) {
			auto QT = PTP->getPointeeType();
			auto l_TP = QT.getTypePtr();
			auto type_str = clang::QualType::getAsString(QT.split());

			stack.push_back(CIndirectionState("native pointer", "native pointer"));

			return populateTypeIndirectionStack(stack, *l_TP, depth+1);
		} else {
			assert(false);
		}
	}
	return stack;
}

std::vector<std::string>& populateStmtIndirectionStack(std::vector<std::string>& stack, const clang::Stmt& stmt, int depth = 0) {
	const clang::Stmt* ST = &stmt;
	auto stmt_class = ST->getStmtClass();
	if (clang::Stmt::StmtClass::ArraySubscriptExprClass == stmt_class) {
		auto ASE = static_cast<const clang::ArraySubscriptExpr*>(ST);
		auto BE = ASE->getBase();
		stack.push_back("ArraySubscriptExpr");
	} else if (clang::Stmt::StmtClass::UnaryOperatorClass == stmt_class) {
		auto UO = static_cast<const clang::UnaryOperator*>(ST);
		if (UO) {
			if (clang::UnaryOperatorKind::UO_Deref == UO->getOpcode()) {
				stack.push_back("Deref");
			}
		} else {
			assert(false);
		}
	} else if(clang::Stmt::StmtClass::DeclRefExprClass == stmt_class) {
		auto DRE = static_cast<const clang::DeclRefExpr*>(ST);
		if (DRE) {
			;
		} else {
			assert(false);
		}
	} else if(clang::Stmt::StmtClass::MemberExprClass == stmt_class) {
		auto ME = static_cast<const clang::MemberExpr*>(ST);
		if (ME) {
			;
		} else {
			assert(false);
		}
	}
	auto child_iter = ST->child_begin();
	if (child_iter != ST->child_end()) {
		return populateStmtIndirectionStack(stack, *(*child_iter), depth+1);
	} else {
		return stack;
	}
}

static std::string IPointerFromPointerDecl(const DeclaratorDecl* DD, Rewriter &Rewrite, const std::string& notes_str = "") {
	std::string retval;

	assert(DD);
	auto DDSR = nice_source_range(DD->getSourceRange(), Rewrite);
	std::string decl_source_text = Rewrite.getRewrittenText(DDSR);

	QualType QT = DD->getType();

	const clang::Type* TP = QT.getTypePtr();

	auto type_str = clang::QualType::getAsString(QT.split());
	clang::StorageDuration storage_duration = clang::StorageDuration::SD_Automatic;
	bool has_dynamic_storage_duration = false;
	bool is_a_temporary = false;
	bool is_static = false;
	bool is_a_function_parameter = false;
	std::string initialization_expr_str;

	auto FD = dynamic_cast<const clang::FieldDecl *>(DD);
	auto VD = dynamic_cast<const clang::VarDecl *>(DD);
	if (VD) {
		storage_duration = VD->getStorageDuration();
		has_dynamic_storage_duration = (clang::StorageDuration::SD_Dynamic == storage_duration);
		is_a_temporary = (clang::StorageDuration::SD_FullExpression == storage_duration);
		is_static = (clang::StorageDuration::SD_Static == storage_duration);
		is_a_function_parameter = (VD->isLocalVarDeclOrParm() && (!VD->isLocalVarDecl()));

		auto pInitExpr = VD->getInit();
		if (VD->hasInit() && pInitExpr) {
			auto init_expr_source_range = nice_source_range(pInitExpr->getSourceRange(), Rewrite);
			initialization_expr_str = Rewrite.getRewrittenText(init_expr_source_range);
		}
	} else if (FD) {
		/* Just placeholder code for now. Haven't thought about this case yet. */
		auto pInitExpr = FD->getInClassInitializer();
		if (FD->hasInClassInitializer() && pInitExpr) {
			auto init_expr_source_range = nice_source_range(pInitExpr->getSourceRange(), Rewrite);
			initialization_expr_str = Rewrite.getRewrittenText(init_expr_source_range);
		}
	}

	auto variable_name = DD->getNameAsString();
	std::string identifier_name_str;
	auto pIdentifier = DD->getIdentifier();
	if (pIdentifier) {
		identifier_name_str = pIdentifier->getName();
	}

	bool replacement_code_generated = false;
	if ("array element" == notes_str) {
		if (TP->isArrayType()) {
			std::string array_size_str;
			bool invalid_flag = false;
			size_t array_size_str_start_index = decl_source_text.size();
			size_t array_size_str_end_index = 0;
			for (size_t index1 = 0; index1 < decl_source_text.size(); index1 += 1) {
				if ('[' == decl_source_text[index1]) {
					array_size_str_start_index = index1 + 1;
					break;
				}
			}
			if (array_size_str_start_index >= decl_source_text.size()) {
				invalid_flag = true;
			}
			for (size_t index1 = decl_source_text.size(); index1 > 1; index1 -= 1) {
				if (']' == decl_source_text[index1 - 1]) {
					array_size_str_end_index = index1 - 1;
					break;
				}
			}
			if (array_size_str_end_index == 0) {
				invalid_flag = true;
			}
			if (invalid_flag || (array_size_str_start_index >= array_size_str_end_index)) {
				invalid_flag = true;
			} else {
				array_size_str = decl_source_text.substr(array_size_str_start_index, array_size_str_end_index - array_size_str_start_index);

				const clang::ArrayType* ATP = TP->getAsArrayTypeUnsafe();
				if (ATP) {
					QT = ATP->getElementType();
					TP = QT.getTypePtr();
					type_str = clang::QualType::getAsString(QT.split());
				} else {
					assert(false);
				}

				auto TPP = static_cast<const clang::PointerType*>(TP);
				if (TPP) {
					auto target_type = TPP->getPointeeType();

					auto splitQualType = target_type.split();
					auto type_str = clang::QualType::getAsString(splitQualType);

					if (("char" != type_str) && ("const char" != type_str)) {
						std::string replacement_code;
						if (is_static) {
							replacement_code += "static ";
						}
						replacement_code += "mse::mstd::array<mse::TIPointerWithBundledVector<";
						replacement_code += type_str;
						replacement_code += ">, ";
						replacement_code += array_size_str;
						replacement_code += "> ";
						replacement_code += variable_name;

						if (("" != initialization_expr_str) && false) {
							replacement_code += " = ";
							replacement_code += initialization_expr_str;
						}
						retval = replacement_code;
						replacement_code_generated = true;
					}
				}
			}
		} else if (TP->isPointerType()) {

			auto PTP = static_cast<const clang::PointerType*>(TP);
			if (PTP) {
				QT = PTP->getPointeeType();
				TP = QT.getTypePtr();
				type_str = clang::QualType::getAsString(QT.split());
			} else {
				assert(false);
			}

			auto TPP = static_cast<const clang::PointerType*>(TP);
			if (TPP) {
				auto target_type = TPP->getPointeeType();

				auto splitQualType = target_type.split();
				auto type_str = clang::QualType::getAsString(splitQualType);

				if (("char" != type_str) && ("const char" != type_str)) {

					std::string replacement_code;
					if (is_static) {
						replacement_code += "static ";
					}
					replacement_code += "mse::TIPointerWithBundledVector<mse::TIPointerWithBundledVector<";
					replacement_code += type_str;
					replacement_code += "> ";
					replacement_code += "> ";
					replacement_code += variable_name;

					if (("" != initialization_expr_str) && false) {
						replacement_code += " = ";
						replacement_code += initialization_expr_str;
					}
					retval = replacement_code;
					replacement_code_generated = true;
				}
			}
		} else {
			assert(false);
		}
	} else if ("dereference" == notes_str) {
		if (TP->isPointerType()) {
			auto PTP = static_cast<const clang::PointerType*>(TP);
			if (PTP) {
				QT = PTP->getPointeeType();
				TP = QT.getTypePtr();
				type_str = clang::QualType::getAsString(QT.split());
			} else {
				assert(false);
			}

			auto TPP = static_cast<const clang::PointerType*>(TP);
			if (TPP) {
				auto target_type = TPP->getPointeeType();

				auto splitQualType = target_type.split();
				auto type_str = clang::QualType::getAsString(splitQualType);

				if (("char" != type_str) && ("const char" != type_str)) {

					std::string replacement_code;
					if (is_static) {
						replacement_code += "static ";
					}
					replacement_code += "mse::TAnyPointer<mse::TIPointerWithBundledVector<";
					replacement_code += type_str;
					replacement_code += "> ";
					replacement_code += "> ";
					replacement_code += variable_name;

					if (("" != initialization_expr_str) && false) {
						replacement_code += " = ";
						replacement_code += initialization_expr_str;
					}
					retval = replacement_code;
					replacement_code_generated = true;
				}
			}
		} else {
			assert(false);
		}
	} else if (TP->isPointerType()) {
		auto TPP = static_cast<const clang::PointerType*>(TP);
		if (TPP) {
			auto target_type = TPP->getPointeeType();

			auto splitQualType = target_type.split();
			auto type_str = clang::QualType::getAsString(splitQualType);

			if (("char" != type_str) && ("const char" != type_str)) {
				std::string replacement_code;
				if (is_static) {
					replacement_code += "static ";
				}
				replacement_code += "mse::TIPointerWithBundledVector<";
				replacement_code += type_str;
				replacement_code += "> ";
				replacement_code += variable_name;

				if (("" != initialization_expr_str) && false) {
					replacement_code += " = ";
					replacement_code += initialization_expr_str;
				}
				retval = replacement_code;
				replacement_code_generated = true;
			} else {
				int q = 3;
			}
		} else {
			assert(false);
			int q = 1;
		}
	}

	if (!replacement_code_generated) {
		auto splitQualType = QT.split();
		auto type_str = clang::QualType::getAsString(splitQualType);

		std::string replacement_code;
		if (is_static) {
			replacement_code += "static ";
		}
		replacement_code += type_str;
		replacement_code += " ";
		replacement_code += variable_name;

		if ("" != initialization_expr_str) {
			replacement_code += " = ";
			replacement_code += initialization_expr_str;
		}
		retval = replacement_code;
		retval = decl_source_text;
		replacement_code_generated = true;
	}
	return retval;
}

class MCSSSMalloc : public MatchFinder::MatchCallback
{
public:
	MCSSSMalloc (Rewriter &Rewrite, CState1& state1)
: Rewrite(Rewrite), m_state1(state1) {}

	virtual void run(const MatchFinder::MatchResult &MR)
	{
		const BinaryOperator* BO = MR.Nodes.getNodeAs<clang::BinaryOperator>("mcsssmalloc1");
		const CallExpr* CE = MR.Nodes.getNodeAs<clang::CallExpr>("mcsssmalloc2");
		const DeclRefExpr* DRE = MR.Nodes.getNodeAs<clang::DeclRefExpr>("mcsssmalloc3");
		const MemberExpr* ME = MR.Nodes.getNodeAs<clang::MemberExpr>("mcsssmalloc4");
		const ArraySubscriptExpr* ASE = MR.Nodes.getNodeAs<clang::ArraySubscriptExpr>("mcsssmalloc5");
		const UnaryOperator* UO = MR.Nodes.getNodeAs<clang::UnaryOperator>("mcsssmalloc6");

		if ((BO != nullptr) && (CE != nullptr) && (DRE != nullptr))
		{
			auto BOSR = nice_source_range(BO->getSourceRange(), Rewrite);
			SourceLocation BOSL = BOSR.getBegin();
			SourceLocation BOSLE = BOSR.getEnd();

			ASTContext *const ASTC = MR.Context;
			FullSourceLoc FBOSL = ASTC->getFullLoc(BOSL);

			SourceManager &SM = ASTC->getSourceManager();

			auto source_location_str = BOSL.printToString(*MR.SourceManager);
			std::string source_text;
			if (BOSL.isValid() && BOSLE.isValid()) {
				source_text = Rewrite.getRewrittenText(SourceRange(BOSL, BOSLE));
			} else {
				return;
			}
			if (std::string::npos != source_location_str.find("526")) {
				int q = 5;
			}

			if (filtered_out_by_location(MR, BOSL)) {
				return void();
			}

			auto function_decl = CE->getDirectCallee();
			auto num_args = CE->getNumArgs();
			if (function_decl && ((1 == num_args) || (2 == num_args))) {
				std::string function_name = function_decl->getNameAsString();
				static const std::string alloc_str = "alloc";
				static const std::string realloc_str = "realloc";
				auto lc_function_name = tolowerstr(function_name);
				bool ends_with_alloc = ((lc_function_name.size() >= alloc_str.size())
						&& (0 == lc_function_name.compare(lc_function_name.size() - alloc_str.size(), alloc_str.size(), alloc_str)));
				bool ends_with_realloc = (ends_with_alloc && (lc_function_name.size() >= realloc_str.size())
						&& (0 == lc_function_name.compare(lc_function_name.size() - realloc_str.size(), realloc_str.size(), realloc_str)));
				bool still_potentially_valid1 = (ends_with_alloc && (1 == num_args)) || (ends_with_realloc && (2 == num_args));
				if (still_potentially_valid1) {
					auto iter = CE->arg_begin();
					if (ends_with_realloc) {
						iter++;
					}
					bool argIsIntegerType = false;
					if (*iter) {
						argIsIntegerType = (*iter)->getType().split().asPair().first->isIntegerType();
					}
					if (argIsIntegerType) {
						auto arg_source_range = nice_source_range((*iter)->getSourceRange(), Rewrite);
						std::string arg_source_text;
						if (arg_source_range.isValid()) {
							arg_source_text = Rewrite.getRewrittenText(arg_source_range);
							//auto arg_source_text_sans_ws = with_whitespace_removed(arg_source_text);

							bool asterisk_found = false;
							auto sizeof_start_index = arg_source_text.find("sizeof(");
							if (std::string::npos != sizeof_start_index) {
								auto sizeof_end_index = arg_source_text.find(")", sizeof_start_index);
								if (std::string::npos != sizeof_end_index) {
									assert(sizeof_end_index > sizeof_start_index);
									std::string before_str = arg_source_text.substr(0, sizeof_start_index);
									std::string after_str;
									if (sizeof_end_index + 1 < arg_source_text.size()) {
										after_str = arg_source_text.substr(sizeof_end_index + 1);
									}

									auto index = before_str.size() - 1;
									while (0 <= index) {
										if ('*' == before_str[index]) {
											asterisk_found = true;
										}
										if (!std::isspace(before_str[index])) {
											break;
										}

										index -= 1;
									}
									if (asterisk_found) {
										before_str = before_str.substr(0, index);
									} else {
										size_t index2 = 0;
										while (after_str.size() > index2) {
											if ('*' == after_str[index2]) {
												asterisk_found = true;
											}
											if (!std::isspace(after_str[index2])) {
												break;
											}

											index2 += 1;
										}
										if (asterisk_found) {
											after_str = after_str.substr(index2 + 1);
										}
									}
								}
							}
							if (true || asterisk_found) {
								/* The argument is in the form "something * sizeof(something_else)" or
								 * "sizeof(something) * something_else". So we're just going to assume that
								 * this is an instance of an array being allocated. */
								std::string num_elements_text/* = before_str + after_str*/;
								QualType QT;
								std::string element_type_str;
								clang::SourceRange decl_source_range;
								std::string declaration_replacement_code;
								std::string variable_name;
								std::string bo_replacement_code;
								const clang::DeclaratorDecl* DD = nullptr;

								auto LHS = BO->getLHS();
								auto lhs_QT = LHS->getType();
								auto lhs_children = LHS->children();
								for (const auto& child : lhs_children) {
									auto stmt_class = child->getStmtClass();
									clang::Stmt::StmtClass a;
								}

								auto decl = DRE->getDecl();
								DD = dynamic_cast<const DeclaratorDecl*>(decl);
								auto VD = dynamic_cast<const VarDecl*>(decl);

								const clang::FieldDecl* FD = nullptr;
								if (nullptr != ME) {
									auto member_decl = ME->getMemberDecl();
									FD = dynamic_cast<const clang::FieldDecl*>(ME->getMemberDecl());
								}
								if (nullptr != FD) {
									DD = FD;
								} else if (nullptr != VD) {
									DD = VD;
								} else {
									int q = 7;
								}

								if (nullptr != DD) {
									auto decl_source_range = nice_source_range(DD->getSourceRange(), Rewrite);
									auto decl_source_location_str = decl_source_range.getBegin().printToString(*MR.SourceManager);
									std::string decl_source_text;
									if (decl_source_range.isValid()) {
										decl_source_text = Rewrite.getRewrittenText(decl_source_range);
									} else {
										return;
									}
									QT = DD->getType();
									const clang::Type* TP = QT.getTypePtr();
									variable_name = DD->getNameAsString();

									auto qualified_name = DD->getQualifiedNameAsString();
									static const std::string mse_namespace_str1 = "mse::";
									static const std::string mse_namespace_str2 = "::mse::";
									if ((0 == qualified_name.compare(0, mse_namespace_str1.size(), mse_namespace_str1))
											|| (0 == qualified_name.compare(0, mse_namespace_str2.size(), mse_namespace_str2))) {
										return;
									}

									std::string notes_str;
									if (ASE != nullptr) {
										notes_str = "array element";
										if (TP->isArrayType()) {
											const clang::ArrayType* ATP = TP->getAsArrayTypeUnsafe();
											if (ATP) {
												QT = ATP->getElementType();
												TP = QT.getTypePtr();
												//type_str = clang::QualType::getAsString(QT.split());
											} else {
												assert(false);
											}
										} else if (TP->isPointerType()) {
											auto PTP = static_cast<const clang::PointerType*>(TP);
											if (PTP) {
												QT = PTP->getPointeeType();
												TP = QT.getTypePtr();
												//type_str = clang::QualType::getAsString(QT.split());
											} else {
												assert(false);
											}
										}
									} else if (UO != nullptr) {
										notes_str = "dereference";
										if (TP->isPointerType()) {
											auto PTP = static_cast<const clang::PointerType*>(TP);
											if (PTP) {
												QT = PTP->getPointeeType();
												TP = QT.getTypePtr();
												//type_str = clang::QualType::getAsString(QT.split());
											} else {
												assert(false);
											}
										}
									}
									auto decls = IndividualDeclaratorDecls(DD, Rewrite);
									if ((1 <= decls.size()) && (decls.back() == DD)) {
										for (const auto& decl : decls) {
											declaration_replacement_code += IPointerFromPointerDecl(decl, Rewrite, notes_str);
											declaration_replacement_code += "; \n";
										}
										if (declaration_replacement_code.size() >= 3) {
											declaration_replacement_code = declaration_replacement_code.substr(0, declaration_replacement_code.size() - 3);
										}
									} else {
										int q = 7;
									}

									if (TP->isArrayType()) {
										auto ATP = static_cast<const clang::ArrayType*>(TP);
										assert(nullptr != ATP);
										auto element_type = ATP->getElementType();
										auto elementSplitQualType = element_type.split();
										element_type_str = clang::QualType::getAsString(elementSplitQualType);
									} else if (TP->isPointerType()) {
										auto TPP = static_cast<const clang::PointerType*>(TP);
										assert(nullptr != TPP);
										auto target_type = TPP->getPointeeType();
										auto splitQualType = target_type.split();
										auto type_str = clang::QualType::getAsString(splitQualType);
										if (("char" != type_str) && ("const char" != type_str)) {
											element_type_str = type_str;
										}
									}
									if ("" != element_type_str) {
										num_elements_text = "(";
										num_elements_text += arg_source_text;
										num_elements_text += ") / sizeof(";
										num_elements_text += element_type_str;
										num_elements_text += ")";

										auto lhs = BO->getLHS();
										auto lhs_source_range = nice_source_range(lhs->getSourceRange(), Rewrite);
										auto lhs_source_text = Rewrite.getRewrittenText(lhs_source_range);
										bo_replacement_code += lhs_source_text;
										bo_replacement_code += ".resize(";
										bo_replacement_code += num_elements_text;
										bo_replacement_code += ")";

										auto decl_source_location_str = decl_source_range.getBegin().printToString(*MR.SourceManager);
										std::string decl_source_text;
										if (decl_source_range.isValid()) {
											decl_source_text = Rewrite.getRewrittenText(decl_source_range);
										} else {
											return;
										}

										auto BOSR = clang::SourceRange(BOSL, BOSLE);
										if (ConvertToSCPP && decl_source_range.isValid() && (BOSR.isValid())) {
											auto cr_shptr = std::make_shared<CMallocArrayReplacementAction>(Rewrite, MR, *DD, BO, bo_replacement_code, declaration_replacement_code);

											bool already_determined_to_be_an_array = false;
											for (const auto& decl : decls) {
												auto iter = m_state1.m_decl_replacement_action_records_log.find(decl);
												if (m_state1.m_decl_replacement_action_records_log.end() != iter) {
													if (("pointer to random access iterator" == (*iter).action_species())
															|| ("pointer targeting heap allocated array to mse vector iterator" == (*iter).action_species())) {
														already_determined_to_be_an_array = true;
													}
												}
											}
											if (already_determined_to_be_an_array) {
												(*cr_shptr).do_replacement(m_state1);
											} else {
												m_state1.m_array_contingent_replacement_map.insert(cr_shptr);
											}
										} else {
											int q = 7;
										}
									}
								}
								int q = 5;
							}
						} else {
							int q = 5;
						}
						int q = 5;
					}
				}

			}
		}
	}

private:
	Rewriter &Rewrite;
	CState1& m_state1;
};

class MCSSSMallocInitializer : public MatchFinder::MatchCallback
{
public:
	MCSSSMallocInitializer (Rewriter &Rewrite, CState1& state1)
: Rewrite(Rewrite), m_state1(state1) {}

	virtual void run(const MatchFinder::MatchResult &MR)
	{
		const DeclStmt* DS = MR.Nodes.getNodeAs<clang::DeclStmt>("mcsssmallocinitializer1");
		const CallExpr* CE = MR.Nodes.getNodeAs<clang::CallExpr>("mcsssmallocinitializer2");
		const DeclaratorDecl* DD = MR.Nodes.getNodeAs<clang::DeclaratorDecl>("mcsssmallocinitializer3");

		if ((DS != nullptr) && (CE != nullptr) && (DD != nullptr))
		{
			auto DSSR = nice_source_range(DS->getSourceRange(), Rewrite);
			SourceLocation DSSL = DSSR.getBegin();
			SourceLocation DSSLE = DSSR.getEnd();

			ASTContext *const ASTC = MR.Context;
			FullSourceLoc FDSSL = ASTC->getFullLoc(DSSL);

			SourceManager &SM = ASTC->getSourceManager();

			auto source_location_str = DSSL.printToString(*MR.SourceManager);
			std::string source_text;
			if (DSSL.isValid() && DSSLE.isValid()) {
				source_text = Rewrite.getRewrittenText(SourceRange(DSSL, DSSLE));
			} else {
				return;
			}

			if (filtered_out_by_location(MR, DSSL)) {
				return void();
			}

			auto function_decl = CE->getDirectCallee();
			auto num_args = CE->getNumArgs();
			if (function_decl && ((1 == num_args) || (2 == num_args))) {
				std::string function_name = function_decl->getNameAsString();
				static const std::string alloc_str = "alloc";
				static const std::string realloc_str = "realloc";
				auto lc_function_name = tolowerstr(function_name);
				bool ends_with_alloc = ((lc_function_name.size() >= alloc_str.size())
						&& (0 == lc_function_name.compare(lc_function_name.size() - alloc_str.size(), alloc_str.size(), alloc_str)));
				bool ends_with_realloc = (ends_with_alloc && (lc_function_name.size() >= realloc_str.size())
						&& (0 == lc_function_name.compare(lc_function_name.size() - realloc_str.size(), realloc_str.size(), realloc_str)));
				bool still_potentially_valid1 = (ends_with_alloc && (1 == num_args)) || (ends_with_realloc && (2 == num_args));
				if (still_potentially_valid1) {
					auto iter = CE->arg_begin();
					if (ends_with_realloc) {
						iter++;
					}
					{
						auto arg_source_range = nice_source_range((*iter)->getSourceRange(), Rewrite);
						std::string arg_source_text;
						if (arg_source_range.isValid()) {
							arg_source_text = Rewrite.getRewrittenText(arg_source_range);
							//auto arg_source_text_sans_ws = with_whitespace_removed(arg_source_text);
							auto sizeof_start_index = arg_source_text.find("sizeof(");
							if (std::string::npos != sizeof_start_index) {
								auto sizeof_end_index = arg_source_text.find(")", sizeof_start_index);
								if (std::string::npos != sizeof_end_index) {
									assert(sizeof_end_index > sizeof_start_index);
									std::string before_str = arg_source_text.substr(0, sizeof_start_index);
									std::string after_str;
									if (sizeof_end_index + 1 < arg_source_text.size()) {
										after_str = arg_source_text.substr(sizeof_end_index + 1);
									}

									bool asterisk_found = false;
									auto index = before_str.size() - 1;
									while (0 <= index) {
										if ('*' == before_str[index]) {
											asterisk_found = true;
										}
										if (!std::isspace(before_str[index])) {
											break;
										}

										index -= 1;
									}
									if (asterisk_found) {
										before_str = before_str.substr(0, index);
									} else {
										size_t index2 = 0;
										while (after_str.size() > index2) {
											if ('*' == after_str[index2]) {
												asterisk_found = true;
											}
											if (!std::isspace(after_str[index2])) {
												break;
											}

											index2 += 1;
										}
										if (asterisk_found) {
											after_str = after_str.substr(index2 + 1);
										}
									}
									if (asterisk_found) {
										/* The argument is in the form "something * sizeof(something_else)" or
										 * "sizeof(something) * something_else". So we're just going to assume that
										 * this is an instance of an array being allocated. */
										std::string num_elements_text = before_str + after_str;
										QualType QT;
										std::string element_type_str;
										clang::SourceRange decl_source_range;
										std::string declaration_replacement_code;
										std::string variable_name;

										if (nullptr != DD) {
											auto decl_source_range = nice_source_range(DD->getSourceRange(), Rewrite);
											auto decl_source_location_str = decl_source_range.getBegin().printToString(*MR.SourceManager);
											std::string decl_source_text;
											if (decl_source_range.isValid()) {
												decl_source_text = Rewrite.getRewrittenText(decl_source_range);
											} else {
												return;
											}
											QT = DD->getType();
											variable_name = DD->getNameAsString();

											auto qualified_name = DD->getQualifiedNameAsString();
											static const std::string mse_namespace_str1 = "mse::";
											static const std::string mse_namespace_str2 = "::mse::";
											if ((0 == qualified_name.compare(0, mse_namespace_str1.size(), mse_namespace_str1))
													|| (0 == qualified_name.compare(0, mse_namespace_str2.size(), mse_namespace_str2))) {
												return;
											}

											auto decls = IndividualDeclaratorDecls(DD, Rewrite);
											if ((1 <= decls.size()) && (decls.back() == DD)) {
												for (const auto& decl : decls) {
													declaration_replacement_code += IPointerFromPointerDecl(decl, Rewrite);
													declaration_replacement_code += "(";
													declaration_replacement_code += num_elements_text;
													declaration_replacement_code += "); \n";
												}
												if (declaration_replacement_code.size() >= 3) {
													declaration_replacement_code = declaration_replacement_code.substr(0, declaration_replacement_code.size() - 3);
												}
											} else {
												int q = 7;
											}

											const clang::Type* TP = QT.getTypePtr();

											if (TP->isArrayType()) {
												auto ATP = static_cast<const clang::ArrayType*>(TP);
												assert(nullptr != ATP);
												auto element_type = ATP->getElementType();
												auto elementSplitQualType = element_type.split();
												element_type_str = clang::QualType::getAsString(elementSplitQualType);
											} else if (TP->isPointerType()) {
												auto TPP = static_cast<const clang::PointerType*>(TP);
												assert(nullptr != TPP);
												auto target_type = TPP->getPointeeType();
												auto splitQualType = target_type.split();
												auto type_str = clang::QualType::getAsString(splitQualType);
												if (("char" != type_str) && ("const char" != type_str)) {
													element_type_str = type_str;
												}
											}
											if ("" != element_type_str) {
												auto decl_source_location_str = decl_source_range.getBegin().printToString(*MR.SourceManager);
												std::string decl_source_text;
												if (decl_source_range.isValid()) {
													decl_source_text = Rewrite.getRewrittenText(decl_source_range);
												} else {
													return;
												}

												auto DSSR = clang::SourceRange(DSSL, DSSLE);
												if (ConvertToSCPP && decl_source_range.isValid() && (DSSR.isValid())) {
													if ((3 <= declaration_replacement_code.size())) {
														bool already_replaced_flag = false;
														size_t replacement_text_length = 0;
														for (const auto& decl : decls) {
															auto iter = m_state1.m_decl_replacement_action_records_log.find(decl);
															if (m_state1.m_decl_replacement_action_records_log.end() != iter) {
																/* This declaration had already been replaced. We'll need to "undo"
																 * the replacement. */
																already_replaced_flag = true;
																if ((*iter).replacement_text().size() > replacement_text_length) {
																	replacement_text_length = (*iter).replacement_text().size();
																}
																m_state1.m_decl_replacement_action_records_log.erase(iter);
															}

															CDeclReplacementActionRecord action_record(Rewrite, *decl, declaration_replacement_code, "pointer targeting heap allocated array to mse vector iterator");
															m_state1.m_decl_replacement_action_records_log.push_back(action_record);
															m_state1.m_dynamic_array_contingent_replacement_map.do_and_dispose_matching_replacements(m_state1, *decl);
														}
														auto adj_decl_source_range = decl_source_range;
														if (already_replaced_flag) {
															adj_decl_source_range = clang::SourceRange(decl_source_range.getBegin(), decl_source_range.getBegin().getLocWithOffset(replacement_text_length));
														}
														auto res = Rewrite.ReplaceText(decl_source_range, declaration_replacement_code);
													}
													int q = 3;
												} else {
													int q = 7;
												}
											}
										}
										int q = 5;
									}
								}
							}
						} else {
							int q = 5;
						}
						int q = 5;
					}
				}

			}
		}
	}

private:
	Rewriter &Rewrite;
	CState1& m_state1;
};

class MCSSSFree : public MatchFinder::MatchCallback
{
public:
	MCSSSFree (Rewriter &Rewrite, CState1& state1)
: Rewrite(Rewrite), m_state1(state1) {}

	virtual void run(const MatchFinder::MatchResult &MR)
	{
		const CallExpr* CE = MR.Nodes.getNodeAs<clang::CallExpr>("mcsssfree1");
		const DeclRefExpr* DRE = MR.Nodes.getNodeAs<clang::DeclRefExpr>("mcsssfree2");
		const MemberExpr* ME = MR.Nodes.getNodeAs<clang::MemberExpr>("mcsssfree3");

		if ((CE != nullptr) && (DRE != nullptr))
		{
			auto CESR = nice_source_range(CE->getSourceRange(), Rewrite);
			SourceLocation CESL = CESR.getBegin();
			SourceLocation CESLE = CESR.getEnd();

			ASTContext *const ASTC = MR.Context;
			FullSourceLoc FCESL = ASTC->getFullLoc(CESL);

			SourceManager &SM = ASTC->getSourceManager();

			auto source_location_str = CESL.printToString(*MR.SourceManager);
			std::string source_text;
			if (CESL.isValid() && CESLE.isValid()) {
				source_text = Rewrite.getRewrittenText(SourceRange(CESL, CESLE));
			} else {
				return;
			}

			if (filtered_out_by_location(MR, CESL)) {
				return void();
			}

			auto function_decl = CE->getDirectCallee();
			auto num_args = CE->getNumArgs();
			if (function_decl && (1 == num_args)) {
				{
					std::string function_name = function_decl->getNameAsString();
					static const std::string free_str = "free";
					auto lc_function_name = tolowerstr(function_name);
					bool ends_with_free = ((lc_function_name.size() >= free_str.size())
							&& (0 == lc_function_name.compare(lc_function_name.size() - free_str.size(), free_str.size(), free_str)));
					if (ends_with_free) {
						auto iter = CE->arg_begin();
						assert((*iter)->getType().getTypePtrOrNull());
						auto arg_source_range = nice_source_range((*iter)->getSourceRange(), Rewrite);
						std::string arg_source_text;
						if (arg_source_range.isValid()) {
							arg_source_text = Rewrite.getRewrittenText(arg_source_range);
							//auto arg_source_text_sans_ws = with_whitespace_removed(arg_source_text);
							QualType QT;
							std::string element_type_str;
							clang::SourceRange decl_source_range;
							std::string variable_name;
							std::string ce_replacement_code;
							const clang::DeclaratorDecl* DD = nullptr;

							auto decl = DRE->getDecl();
							DD = dynamic_cast<const DeclaratorDecl*>(decl);
							auto VD = dynamic_cast<const VarDecl*>(decl);

							const clang::FieldDecl* FD = nullptr;
							if (nullptr != ME) {
								auto member_decl = ME->getMemberDecl();
								FD = dynamic_cast<const clang::FieldDecl*>(ME->getMemberDecl());
							}
							if (nullptr != FD) {
								DD = FD;
							} else if (nullptr != VD) {
								DD = VD;
							} else {
								int q = 7;
							}

							if (nullptr != DD) {
								auto decl_source_range = nice_source_range(DD->getSourceRange(), Rewrite);
								auto decl_source_location_str = decl_source_range.getBegin().printToString(*MR.SourceManager);
								std::string decl_source_text;
								if (decl_source_range.isValid()) {
									decl_source_text = Rewrite.getRewrittenText(decl_source_range);
								} else {
									return;
								}
								QT = DD->getType();
								variable_name = DD->getNameAsString();

								auto qualified_name = DD->getQualifiedNameAsString();
								static const std::string mse_namespace_str1 = "mse::";
								static const std::string mse_namespace_str2 = "::mse::";
								if ((0 == qualified_name.compare(0, mse_namespace_str1.size(), mse_namespace_str1))
										|| (0 == qualified_name.compare(0, mse_namespace_str2.size(), mse_namespace_str2))) {
									return;
								}

								auto iter = m_state1.m_decl_replacement_action_records_log.find(DD/*decl_source_range.getBegin()*/);
								if (m_state1.m_decl_replacement_action_records_log.end() != iter) {
									if ("pointer targeting heap allocated array to mse vector iterator" == (*iter).action_species()) {
										ce_replacement_code = arg_source_text;
										ce_replacement_code += ".resize(0)";

										auto CESR = clang::SourceRange(CESL, CESLE);
										if (ConvertToSCPP && (CESR.isValid())) {
											auto res2 = Rewrite.ReplaceText(CESR, ce_replacement_code);
											int q = 3;
										} else {
											int q = 7;
										}
									}
								} else {
									auto cr_shptr = std::make_shared<CFreeDynamicArrayReplacementAction>(Rewrite, MR, *DD, CE, DRE, ME);
									m_state1.m_dynamic_array_contingent_replacement_map.insert(cr_shptr);
								}
							}
							int q = 5;
						} else {
							int q = 5;
						}
						int q = 5;
					}
				}

			}
		}
	}

private:
	Rewriter &Rewrite;
	CState1& m_state1;
};

class MCSSSSetToNull : public MatchFinder::MatchCallback
{
public:
	MCSSSSetToNull (Rewriter &Rewrite, CState1& state1)
: Rewrite(Rewrite), m_state1(state1) {}

	virtual void run(const MatchFinder::MatchResult &MR)
	{
		const BinaryOperator* BO = MR.Nodes.getNodeAs<clang::BinaryOperator>("mcssssettonull1");
		const Expr* RHS = nullptr;
		const Expr* LHS = nullptr;
		if (BO != nullptr) {
			RHS = BO->getRHS();
			LHS = BO->getLHS();
		}
		const DeclRefExpr* DRE = MR.Nodes.getNodeAs<clang::DeclRefExpr>("mcssssettonull3");
		const MemberExpr* ME = MR.Nodes.getNodeAs<clang::MemberExpr>("mcssssettonull4");

		if ((BO != nullptr) && (RHS != nullptr) && (LHS != nullptr) && (DRE != nullptr))
		{
			auto BOSR = nice_source_range(BO->getSourceRange(), Rewrite);
			SourceLocation BOSL = BOSR.getBegin();
			SourceLocation BOSLE = BOSR.getEnd();

			ASTContext *const ASTC = MR.Context;
			FullSourceLoc FBOSL = ASTC->getFullLoc(BOSL);

			SourceManager &SM = ASTC->getSourceManager();

			auto source_location_str = BOSL.printToString(*MR.SourceManager);
			std::string source_text;
			if (BOSL.isValid() && BOSLE.isValid()) {
				source_text = Rewrite.getRewrittenText(SourceRange(BOSL, BOSLE));
			} else {
				return;
			}

			if (filtered_out_by_location(MR, BOSL)) {
				return void();
			}

			Expr::NullPointerConstantKind kind = RHS->isNullPointerConstant(*ASTC, Expr::NullPointerConstantValueDependence());
			if (clang::Expr::NPCK_NotNull != kind) {
				auto lhs_source_range = nice_source_range(LHS->getSourceRange(), Rewrite);
				std::string lhs_source_text;
				if (lhs_source_range.isValid()) {
					lhs_source_text = Rewrite.getRewrittenText(lhs_source_range);
					//auto lhs_source_text_sans_ws = with_whitespace_removed(lhs_source_text);

					QualType QT;
					std::string element_type_str;
					clang::SourceRange decl_source_range;
					std::string variable_name;
					std::string bo_replacement_code;
					const clang::DeclaratorDecl* DD = nullptr;

					auto decl = DRE->getDecl();
					DD = dynamic_cast<const DeclaratorDecl*>(decl);
					auto VD = dynamic_cast<const VarDecl*>(decl);

					const clang::FieldDecl* FD = nullptr;
					if (nullptr != ME) {
						auto member_decl = ME->getMemberDecl();
						FD = dynamic_cast<const clang::FieldDecl*>(ME->getMemberDecl());
					}
					if (nullptr != FD) {
						DD = FD;
					} else if (nullptr != VD) {
						DD = VD;
					} else {
						int q = 7;
					}

					if (nullptr != DD) {
						auto decl_source_range = nice_source_range(DD->getSourceRange(), Rewrite);
						auto decl_source_location_str = decl_source_range.getBegin().printToString(*MR.SourceManager);
						std::string decl_source_text;
						if (decl_source_range.isValid()) {
							decl_source_text = Rewrite.getRewrittenText(decl_source_range);
						} else {
							return;
						}
						QT = DD->getType();
						variable_name = DD->getNameAsString();

						auto qualified_name = DD->getQualifiedNameAsString();
						static const std::string mse_namespace_str1 = "mse::";
						static const std::string mse_namespace_str2 = "::mse::";
						if ((0 == qualified_name.compare(0, mse_namespace_str1.size(), mse_namespace_str1))
								|| (0 == qualified_name.compare(0, mse_namespace_str2.size(), mse_namespace_str2))) {
							return;
						}

						if ((DD->getType() == LHS->getType())) {
							auto iter = m_state1.m_decl_replacement_action_records_log.find(DD/*decl_source_range.getBegin()*/);
							if ((m_state1.m_decl_replacement_action_records_log.end() != iter)
									&& ("pointer targeting heap allocated array to mse vector iterator" == (*iter).action_species())
									&& ((*iter).get_ddecl_cptr()->getType() == LHS->getType())) {
								if (true) {
									bo_replacement_code = lhs_source_text;
									bo_replacement_code += ".resize(0)";

									auto BOSR = clang::SourceRange(BOSL, BOSLE);
									if (ConvertToSCPP && (BOSR.isValid())) {
										auto res2 = Rewrite.ReplaceText(BOSR, bo_replacement_code);
										int q = 3;
									} else {
										int q = 7;
									}
								} else {
									int q = 5;
								}
							} else {
								auto cr_shptr = std::make_shared<CSetArrayPointerToNullReplacementAction>(Rewrite, MR, *DD, BO, DRE, ME);
								m_state1.m_dynamic_array_contingent_replacement_map.insert(cr_shptr);
							}
						}
					}
					int q = 5;
				} else {
					int q = 5;
				}
				int q = 5;
			}
		}
	}

private:
	Rewriter &Rewrite;
	CState1& m_state1;
};


/**********************************************************************************************************************/
class MyASTConsumer : public ASTConsumer {

public:
  MyASTConsumer(Rewriter &R) : HandlerForSSSNativePointer(R), HandlerForSSSArrayToPointerDecay(R, m_state1), \
	HandlerForSSSVarDecl(R, m_state1), HandlerForSSSPointerArithmetic(R, m_state1), \
	HandlerForSSSMalloc(R, m_state1), HandlerForSSSMallocInitializer(R, m_state1), \
	HandlerForSSSFree(R, m_state1), HandlerForSSSSetToNull(R, m_state1)
  {
	  Matcher.addMatcher(varDecl(hasType(pointerType())).bind("mcsssnativepointer"), &HandlerForSSSNativePointer);

	  Matcher.addMatcher(castExpr(allOf(hasCastKind(CK_ArrayToPointerDecay), unless(hasParent(arraySubscriptExpr())))).bind("mcsssarraytopointerdecay"), &HandlerForSSSArrayToPointerDecay);

	  Matcher.addMatcher(varDecl().bind("mcsssvardecl"), &HandlerForSSSVarDecl);

	  Matcher.addMatcher(declRefExpr(allOf(hasParent(expr(anyOf( \
	  		unaryOperator(hasOperatorName("++")), unaryOperator(hasOperatorName("--")), \
				binaryOperator(hasOperatorName("+=")), binaryOperator(hasOperatorName("-=")), \
				hasParent(expr(anyOf( \
						binaryOperator(hasOperatorName("+")), binaryOperator(hasOperatorName("+=")), \
						binaryOperator(hasOperatorName("-")), binaryOperator(hasOperatorName("-=")), \
						binaryOperator(hasOperatorName("<=")), binaryOperator(hasOperatorName("<")), \
						binaryOperator(hasOperatorName(">=")), binaryOperator(hasOperatorName(">")), \
						arraySubscriptExpr(), clang::ast_matchers::castExpr(hasParent(arraySubscriptExpr())) \
				)))))), to(declaratorDecl(hasType(pointerType()))))).bind("mcssspointerarithmetic"), &HandlerForSSSPointerArithmetic);

	  Matcher.addMatcher(memberExpr(allOf(hasDescendant(declRefExpr().bind("mcssspointerarithmetic")), hasParent(expr(anyOf( \
	  		unaryOperator(hasOperatorName("++")), unaryOperator(hasOperatorName("--")), \
				binaryOperator(hasOperatorName("+=")), binaryOperator(hasOperatorName("-=")), \
				hasParent(expr(anyOf( \
						binaryOperator(hasOperatorName("+")), binaryOperator(hasOperatorName("+=")), \
						binaryOperator(hasOperatorName("-")), binaryOperator(hasOperatorName("-=")), \
						binaryOperator(hasOperatorName("<=")), binaryOperator(hasOperatorName("<")), \
						binaryOperator(hasOperatorName(">=")), binaryOperator(hasOperatorName(">")), \
						arraySubscriptExpr(), clang::ast_matchers::castExpr(hasParent(arraySubscriptExpr())) \
				)))))), member(hasType(pointerType())))).bind("mcssspointerarithmetic2"), &HandlerForSSSPointerArithmetic);

	  Matcher.addMatcher(binaryOperator(allOf(
	  		hasOperatorName("="),
	  		hasRHS(
	  				anyOf(
	  						cStyleCastExpr(has(callExpr().bind("mcsssmalloc2"))),
							callExpr().bind("mcsssmalloc2")
	  				)
				),
	  		hasLHS(anyOf(
	  				unaryOperator(allOf(hasOperatorName("*"), hasDescendant(memberExpr(expr(hasDescendant(declRefExpr().bind("mcsssmalloc3")))).bind("mcsssmalloc4")))).bind("mcsssmalloc6"),
	  				unaryOperator(allOf(hasOperatorName("*"), hasDescendant(declRefExpr().bind("mcsssmalloc3")))).bind("mcsssmalloc6"),
						arraySubscriptExpr(expr(hasDescendant(memberExpr(expr(hasDescendant(declRefExpr().bind("mcsssmalloc3")))).bind("mcsssmalloc4")))).bind("mcsssmalloc5"),
						arraySubscriptExpr(expr(hasDescendant(declRefExpr().bind("mcsssmalloc3")))).bind("mcsssmalloc5"),
						memberExpr(expr(hasDescendant(declRefExpr().bind("mcsssmalloc3")))).bind("mcsssmalloc4"),
						declRefExpr().bind("mcsssmalloc3"),
						hasDescendant(unaryOperator(allOf(hasOperatorName("*"), hasDescendant(memberExpr(expr(hasDescendant(declRefExpr().bind("mcsssmalloc3")))).bind("mcsssmalloc4")))).bind("mcsssmalloc6")),
						hasDescendant(unaryOperator(allOf(hasOperatorName("*"), hasDescendant(declRefExpr().bind("mcsssmalloc3")))).bind("mcsssmalloc6")),
						hasDescendant(arraySubscriptExpr(expr(hasDescendant(memberExpr(expr(hasDescendant(declRefExpr().bind("mcsssmalloc3")))).bind("mcsssmalloc4")))).bind("mcsssmalloc5")),
						hasDescendant(arraySubscriptExpr(expr(hasDescendant(declRefExpr().bind("mcsssmalloc3")))).bind("mcsssmalloc5")),
						hasDescendant(memberExpr(expr(hasDescendant(declRefExpr().bind("mcsssmalloc3")))).bind("mcsssmalloc4")),
						hasDescendant(declRefExpr().bind("mcsssmalloc3"))
				)),
	  		hasLHS(expr(hasType(pointerType())))
				)).bind("mcsssmalloc1"), &HandlerForSSSMalloc);

	  Matcher.addMatcher(declStmt(hasDescendant(
	  		varDecl(hasInitializer(ignoringImpCasts(
					anyOf(
							cStyleCastExpr(has(callExpr().bind("mcsssmallocinitializer2"))),
							callExpr().bind("mcsssmallocinitializer2")
					)
	  		))).bind("mcsssmallocinitializer3")
				)).bind("mcsssmallocinitializer1"), &HandlerForSSSMallocInitializer);

	  Matcher.addMatcher(
	  		callExpr(allOf(
	  				hasAnyArgument(
	  				expr(anyOf(
								memberExpr(expr(hasDescendant(declRefExpr().bind("mcsssfree2")))).bind("mcsssfree3"),
								hasDescendant(memberExpr(expr(hasDescendant(declRefExpr().bind("mcsssfree2")))).bind("mcsssfree3")),
								hasDescendant(declRefExpr().bind("mcsssfree2"))
						))),
						argumentCountIs(1),
						hasAnyArgument(hasType(pointerType()))
	  		)).bind("mcsssfree1"), &HandlerForSSSFree);

	  Matcher.addMatcher(binaryOperator(allOf(
	  		hasOperatorName("="),
	  		hasLHS(anyOf(
	  				memberExpr(expr(hasDescendant(declRefExpr().bind("mcssssettonull3")))).bind("mcssssettonull4"),
	  				hasDescendant(memberExpr(expr(hasDescendant(declRefExpr().bind("mcssssettonull3")))).bind("mcssssettonull4")),
						hasDescendant(declRefExpr().bind("mcssssettonull3"))
				)),
				hasLHS(expr(hasType(pointerType())))
				)).bind("mcssssettonull1"), &HandlerForSSSSetToNull);
  }

  void HandleTranslationUnit(ASTContext &Context) override 
  {
    Matcher.matchAST(Context);
  }

private:

  CState1 m_state1;

  MCSSSNativePointer HandlerForSSSNativePointer;
  MCSSSArrayToPointerDecay HandlerForSSSArrayToPointerDecay;
  MCSSSVarDecl HandlerForSSSVarDecl;
  MCSSSPointerArithmetic HandlerForSSSPointerArithmetic;
  MCSSSMalloc HandlerForSSSMalloc;
  MCSSSMallocInitializer HandlerForSSSMallocInitializer;
  MCSSSFree HandlerForSSSFree;
  MCSSSSetToNull HandlerForSSSSetToNull;

  MatchFinder Matcher;
};

/**********************************************************************************************************************/
class MyFrontendAction : public ASTFrontendAction 
{
public:
  MyFrontendAction() {}
  ~MyFrontendAction() {
	  if (ConvertToSCPP) {
		  auto res = overwriteChangedFiles();
		  int q = 5;
	  }
  }

  void EndSourceFileAction() override {
    TheRewriter.getEditBuffer(TheRewriter.getSourceMgr().getMainFileID()).write(llvm::outs());
  }

  std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI, StringRef file) override {
    TheRewriter.setSourceMgr(CI.getSourceManager(), CI.getLangOpts());
    return llvm::make_unique<MyASTConsumer>(TheRewriter);
  }

  bool overwriteChangedFiles() {
	  return TheRewriter.overwriteChangedFiles();
  }

private:
  Rewriter TheRewriter;
};
/**********************************************************************************************************************/
/*Main*/
int main(int argc, const char **argv) 
{
  CommonOptionsParser op(argc, argv, MatcherSampleCategory);
  ClangTool Tool(op.getCompilations(), op.getSourcePathList());

  return Tool.run(newFrontendActionFactory<MyFrontendAction>().get());
}
/*last line intentionally left blank*/

