
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
	} else {
		bool filename_is_invalid = false;
		std::string full_path_name = MR.SourceManager->getBufferName(SL, &filename_is_invalid);
		std::string filename = full_path_name;
		auto last_slash_pos = full_path_name.find_last_of('/');
		if (std::string::npos != last_slash_pos) {
			if (last_slash_pos + 1 < full_path_name.size()) {
				filename = full_path_name.substr(last_slash_pos+1);
			} else {
				filename = "";
			}
		}
		static const std::string mse_str = "mse";
		if (0 == filename.compare(0, mse_str.size(), mse_str)) {
			retval = true;
		}
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

/* No longer used. This function extracts the text of individual declarations when multiple
 * pointers are declared in the same declaration statement. */
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

std::string tolowerstr(const std::string& a) {
	std::string retval;
	for (const auto& ch : a) {
		retval += tolower(ch);
	}
	return retval;
}

bool string_begins_with(const std::string& s1, const std::string& prefix) {
	return (0 == s1.compare(0, prefix.length(), prefix));
}

/* This function returns a list of individual declarations contained in the same declaration statement
 * as the given declaration. (eg.: "int a, b = 3, *c;" ) */
static std::vector<const DeclaratorDecl*> IndividualDeclaratorDecls(const DeclaratorDecl* DD, Rewriter &Rewrite) {
	/* There's probably a more efficient way to do this, but this implementation seems to work. */
	std::vector<const DeclaratorDecl*> retval;

	if (!DD) {
		assert(false);
		return retval;
	}
	auto SR = nice_source_range(DD->getSourceRange(), Rewrite);
	SourceLocation SL = SR.getBegin();

	std::string source_text;
	if (SR.isValid()) {
		source_text = Rewrite.getRewrittenText(SR);
	} else {
		return retval;
	}

	auto decl_context = DD->getDeclContext();
	if ((!decl_context) || (!SL.isValid())) {
		assert(false);
		retval.push_back(DD);
	} else {
		for (auto decl_iter = decl_context->decls_begin(); decl_iter != decl_context->decls_end(); decl_iter++) {
			auto decl = (*decl_iter);
			auto l_DD = dynamic_cast<const DeclaratorDecl*>(decl);
			if (l_DD) {
				auto DDSR = nice_source_range(l_DD->getSourceRange(), Rewrite);
				SourceLocation l_SL = DDSR.getBegin();
				if (l_SL == SL) {
					retval.push_back(l_DD);
				}
			}
		}
	}
	if (0 == retval.size()) {
		//assert(false);
		int q = 7;
	}

	return retval;
}

/* This class specifies a declaration and a level of "indirection"(/"dereference") relative to the declared
 * object. For example, given the declaration "int **var1[5];", (*var1) and (**var1) are 1 and 2 "levels of
 * indirection", respectively, relative to var1. */
class CDDeclIndirection {
public:
	CDDeclIndirection(const clang::DeclaratorDecl& ddecl_cref, size_t indirection_level = 0) :
		m_ddecl_cptr(&ddecl_cref), m_indirection_level(indirection_level) {}
	CDDeclIndirection(const CDDeclIndirection&) = default;
	bool operator <(const CDDeclIndirection &rhs) const {
		if (m_ddecl_cptr == rhs.m_ddecl_cptr) {
			return (m_indirection_level < rhs.m_indirection_level);
		} else {
			return (m_ddecl_cptr < rhs.m_ddecl_cptr);
		}
	}

	const clang::DeclaratorDecl* m_ddecl_cptr = nullptr;
	size_t m_indirection_level = 0;
};

class CIndirectionState {
public:
	CIndirectionState(std::string original, std::string current)
		: m_original(original), m_current(current) {}
	CIndirectionState(const CIndirectionState& src) = default;
	std::string m_original;
	std::string m_current;
};

class CIndirectionStateStack : public std::vector<CIndirectionState> {};

/* Given a type and an (empty) CIndirectionStateStack, this function will fill the stack with indications of
 * whether each level of indirection (if any) of the type is of the pointer or the array variety. Pointers
 * can, of course, function as arrays, but context is required to identify those situations. Such identification
 * is not done in this function. It is done elsewhere.  */
const clang::QualType& populateQTypeIndirectionStack(CIndirectionStateStack& stack, const clang::QualType& qtype, int depth = 0) {
	std::string qtype_str = qtype.getAsString();
	auto TP = qtype.getTypePtr();

	if (TP->isArrayType()) {
		auto type_class = qtype->getTypeClass();
		if (clang::Type::Decayed == type_class) {
			int q = 5;
		} else if (clang::Type::ConstantArray == type_class) {
			int q = 5;
		}

		const clang::ArrayType* ATP = TP->getAsArrayTypeUnsafe();
		if (ATP) {
			clang::QualType QT = ATP->getElementType();
			auto l_TP = QT.getTypePtr();
			auto l_type_str = QT.getAsString();

			stack.push_back(CIndirectionState("native array", "native array"));

			return populateQTypeIndirectionStack(stack, QT, depth+1);
		} else {
			assert(false);
		}
	} else if (qtype->isPointerType()) {
		auto type_class = qtype->getTypeClass();
		if (clang::Type::Decayed == type_class) {
			int q = 5;
		} else if (clang::Type::Pointer == type_class) {
			int q = 5;
		}

		if (llvm::isa<const clang::PointerType>(qtype)) {
			auto PQT = llvm::cast<const clang::PointerType>(qtype);
			if (PQT) {
				int q = 5;
			} else {
				int q = 5;
			}
		} else {
			int q = 5;
		}

		clang::QualType QT = qtype->getPointeeType();
		auto l_type_str = QT.getAsString();

		stack.push_back(CIndirectionState("native pointer", "native pointer"));

		return populateQTypeIndirectionStack(stack, QT, depth+1);
	}
	return qtype;
}

/* Given an expression (in the form of a clang::Stmt) and an (empty) (string) stack,
 * this function will fill the stack with indications of whether each level of indirection
 * (if any) (in the expression) is a pointer dereference or an array subscript. */
const clang::Expr* populateStmtIndirectionStack(std::vector<std::string>& stack, const clang::Stmt& stmt, int depth = 0) {
	const clang::Expr* retval = nullptr;
	const clang::Stmt* ST = &stmt;
	auto stmt_class = ST->getStmtClass();
	auto stmt_class_name = ST->getStmtClassName();
	bool process_child_flag = false;
	if (clang::Stmt::StmtClass::ArraySubscriptExprClass == stmt_class) {
		stack.push_back("ArraySubscriptExpr");
		process_child_flag = true;
	} else if (clang::Stmt::StmtClass::UnaryOperatorClass == stmt_class) {
		auto UO = llvm::cast<const clang::UnaryOperator>(ST);
		if (UO) {
			if (clang::UnaryOperatorKind::UO_Deref == UO->getOpcode()) {
				stack.push_back("Deref");
				process_child_flag = true;
			} else {
				auto QT = UO->getType();
				const clang::Type* TP = QT.getTypePtr();
				if (TP && TP->isPointerType()) {
					if ((clang::UnaryOperatorKind::UO_PreInc == UO->getOpcode())
							|| (clang::UnaryOperatorKind::UO_PostInc == UO->getOpcode())
							|| (clang::UnaryOperatorKind::UO_PreDec == UO->getOpcode())
							|| (clang::UnaryOperatorKind::UO_PostDec == UO->getOpcode())) {
						/* Incrementing/decrementing a pointer type is pointer arithmetic and
						 * implies the pointer is being used as an array iterator. */
						/* To do: modify the stack entry to reflect this. */
					}
				}
			}
		} else {
			assert(false);
		}
	} else if ((clang::Stmt::StmtClass::ImplicitCastExprClass == stmt_class)) {
		auto ICE = llvm::cast<const clang::ImplicitCastExpr>(ST);
		if (ICE) {
			auto cast_kind_name = ICE->getCastKindName();
			auto cast_kind = ICE->getCastKind();
			if ((clang::CK_FunctionToPointerDecay == cast_kind)) {
				process_child_flag = false;
			} else {
				if ((clang::CK_ArrayToPointerDecay == cast_kind) || (clang::CK_LValueToRValue == cast_kind)) {
					process_child_flag = true;
				} else {
					process_child_flag = true;
				}
			}
		} else { assert(false); }
	} else if ((clang::Stmt::StmtClass::ParenExprClass == stmt_class)) {
		process_child_flag = true;
	} else if(clang::Stmt::StmtClass::DeclRefExprClass == stmt_class) {
		auto DRE = llvm::cast<const clang::DeclRefExpr>(ST);
		if (DRE) {
			retval = DRE;
			process_child_flag = true;
		} else {
			assert(false);
		}
	} else if(clang::Stmt::StmtClass::MemberExprClass == stmt_class) {
		auto ME = llvm::cast<const clang::MemberExpr>(ST);
		if (ME) {
			retval = ME;
		} else {
			assert(false);
		}
	} else {
		if (0 == depth) {
			int q = 5;
		}
		int q = 5;
	}
	if (process_child_flag) {
		auto child_iter = ST->child_begin();
		if (child_iter != ST->child_end()) {
			if (nullptr != (*child_iter)) {
				const auto noted_stack_size = stack.size();
				auto res = populateStmtIndirectionStack(stack, *(*child_iter), depth+1);
				if ((nullptr == retval) || (stack.size() > noted_stack_size)) {
					retval = res;
				}
			} else {
				assert(false);
			}
		} else {
			int q = 5;
		}
	}
	return retval;
}

void walkTheAST1(const clang::Stmt& stmt, int depth = 0) {
	const clang::Stmt* ST = &stmt;
	auto stmt_class = ST->getStmtClass();
	auto stmt_class_name = ST->getStmtClassName();
	bool process_children_flag = true;
	if (clang::Stmt::StmtClass::ArraySubscriptExprClass == stmt_class) {
		//stack.push_back("ArraySubscriptExpr");
		process_children_flag = true;
	} else if (clang::Stmt::StmtClass::UnaryOperatorClass == stmt_class) {
		auto UO = llvm::cast<const clang::UnaryOperator>(ST);
		if (UO) {
			if (clang::UnaryOperatorKind::UO_Deref == UO->getOpcode()) {
				//stack.push_back("Deref");
				process_children_flag = true;
			} else {
				auto QT = UO->getType();
				const clang::Type* TP = QT.getTypePtr();
				if (TP && TP->isPointerType()) {
					if ((clang::UnaryOperatorKind::UO_PreInc == UO->getOpcode())
							|| (clang::UnaryOperatorKind::UO_PostInc == UO->getOpcode())
							|| (clang::UnaryOperatorKind::UO_PreDec == UO->getOpcode())
							|| (clang::UnaryOperatorKind::UO_PostDec == UO->getOpcode())) {
						/* Incrementing/decrementing a pointer type is pointer arithmetic and
						 * implies the pointer is being used as an array iterator. */
						int q = 5;
					}
				}
			}
		} else {
			assert(false);
		}
	} else if ((clang::Stmt::StmtClass::ImplicitCastExprClass == stmt_class)) {
		auto ICE = llvm::cast<const clang::ImplicitCastExpr>(ST);
		if (ICE) {
			auto cast_kind_name = ICE->getCastKindName();
			auto cast_kind = ICE->getCastKind();
			if ((clang::CK_FunctionToPointerDecay == cast_kind)) {
				process_children_flag = false;
			} else {
				if ((clang::CK_ArrayToPointerDecay == cast_kind) || (clang::CK_LValueToRValue == cast_kind)) {
					process_children_flag = true;
				} else {
					process_children_flag = true;
				}
			}
		} else { assert(false); }
	} else if ((clang::Stmt::StmtClass::ParenExprClass == stmt_class)) {
		process_children_flag = true;
	} else if(clang::Stmt::StmtClass::DeclRefExprClass == stmt_class) {
		auto DRE = llvm::cast<const clang::DeclRefExpr>(ST);
		if (DRE) {
			//retval = DRE;
			process_children_flag = true;
		} else {
			assert(false);
		}
	} else if(clang::Stmt::StmtClass::MemberExprClass == stmt_class) {
		auto ME = llvm::cast<const clang::MemberExpr>(ST);
		if (ME) {
			//retval = ME;
		} else {
			assert(false);
		}
	} else {
		if (0 == depth) {
			int q = 5;
		}
		int q = 5;
	}
	if (process_children_flag) {
		for (auto child_iter = ST->child_begin(); child_iter != ST->child_end(); child_iter++) {
			if (nullptr != (*child_iter)) {
				walkTheAST1(*(*child_iter), depth+1);
			} else {
				assert(false);
			}
		}
	}
	return;
}

class CDDeclConversionState {
public:
	CDDeclConversionState(const clang::DeclaratorDecl& ddecl) : m_ddecl_cptr(&ddecl) {
		QualType QT = ddecl.getType();
		m_direct_qtype = populateQTypeIndirectionStack(m_indirection_state_stack, QT);
		//std::reverse(m_indirection_state_stack.begin(), m_indirection_state_stack.end());
	}
	const DeclaratorDecl* m_ddecl_cptr = nullptr;
	CIndirectionStateStack m_indirection_state_stack;
	clang::QualType m_direct_qtype;
	std::string m_initializer_info_str;
	bool m_original_initialization_has_been_noted = false;
	std::string m_original_initialization_expr_str;
	bool m_original_source_text_has_been_noted = false;
	std::string m_original_source_text_str;
};

class CDDeclConversionStateMap : public std::map<const clang::DeclaratorDecl*, CDDeclConversionState> {
public:
	std::pair<iterator, bool> insert(const clang::DeclaratorDecl& ddecl) {
		value_type item(&ddecl, CDDeclConversionState(ddecl));
		return std::map<const clang::DeclaratorDecl*, CDDeclConversionState>::insert(item);
	}
};


class CState1;

class CReplacementAction {
public:
	virtual ~CReplacementAction() {}
	virtual void do_replacement(CState1& state1) const = 0;
};

class CDDecl2ReplacementAction : public CReplacementAction {
public:
	CDDecl2ReplacementAction(Rewriter &Rewrite, const MatchFinder::MatchResult &MR,
			const CDDeclIndirection& ddecl_indirection) : m_Rewrite(Rewrite), m_MR(MR), m_ddecl_indirection(ddecl_indirection) {}
	virtual ~CDDecl2ReplacementAction() {}

	virtual void do_replacement(CState1& state1) const = 0;
	virtual const clang::DeclaratorDecl* get_ddecl_cptr() const { return m_ddecl_indirection.m_ddecl_cptr; }
	virtual const CDDeclIndirection& ddecl_indirection_cref() const { return m_ddecl_indirection; }

	clang::SourceRange source_range() {
		clang::SourceRange retval = m_ddecl_indirection.m_ddecl_cptr->getSourceRange();
		return retval;
	}
	clang::SourceLocation start_location() {
		clang::SourceLocation retval = source_range().getBegin();
		return retval;
	}
	std::string get_var_name() {
		std::string retval = m_ddecl_indirection.m_ddecl_cptr->getNameAsString();
		return retval;
	}

	Rewriter& m_Rewrite;
	const MatchFinder::MatchResult m_MR;
	CDDeclIndirection m_ddecl_indirection;
};

class CArray2ReplacementAction : public CDDecl2ReplacementAction {
public:
	using CDDecl2ReplacementAction::CDDecl2ReplacementAction;
	virtual ~CArray2ReplacementAction() {}
};

class CMemsetArray2ReplacementAction : public CArray2ReplacementAction {
public:
	CMemsetArray2ReplacementAction(Rewriter &Rewrite, const MatchFinder::MatchResult &MR, const CDDeclIndirection& ddecl_indirection,
			const CallExpr* CE, const std::string& ce_replacement_code) :
				CArray2ReplacementAction(Rewrite, MR, ddecl_indirection), m_CE(CE), m_DD(ddecl_indirection.m_ddecl_cptr),
				m_ce_replacement_code(ce_replacement_code) {}
	virtual ~CMemsetArray2ReplacementAction() {}

	virtual void do_replacement(CState1& state1) const;

	const CallExpr* m_CE = nullptr;
	//const DeclRefExpr* m_DRE = nullptr;
	//const MemberExpr* m_ME = nullptr;
	const DeclaratorDecl* m_DD = nullptr;
	std::string m_ce_replacement_code;
};

class CMemcpyArray2ReplacementAction : public CArray2ReplacementAction {
public:
	CMemcpyArray2ReplacementAction(Rewriter &Rewrite, const MatchFinder::MatchResult &MR, const CDDeclIndirection& ddecl_indirection,
			const CallExpr* CE, const std::string& ce_replacement_code) :
				CArray2ReplacementAction(Rewrite, MR, ddecl_indirection), m_CE(CE), m_DD(ddecl_indirection.m_ddecl_cptr),
				m_ce_replacement_code(ce_replacement_code) {}
	virtual ~CMemcpyArray2ReplacementAction() {}

	virtual void do_replacement(CState1& state1) const;

	const CallExpr* m_CE = nullptr;
	//const DeclRefExpr* m_DRE = nullptr;
	//const MemberExpr* m_ME = nullptr;
	const DeclaratorDecl* m_DD = nullptr;
	std::string m_ce_replacement_code;
};

class CAssignedFromArray2ReplacementAction : public CArray2ReplacementAction {
public:
	CAssignedFromArray2ReplacementAction(Rewriter &Rewrite, const MatchFinder::MatchResult &MR, const CDDeclIndirection& ddecl_indirection,
			const CDDeclIndirection& ddecl_indirection2) :
				CArray2ReplacementAction(Rewrite, MR, ddecl_indirection), m_ddecl_indirection2(ddecl_indirection2) {}
	virtual ~CAssignedFromArray2ReplacementAction() {}

	virtual void do_replacement(CState1& state1) const;

	const CDDeclIndirection m_ddecl_indirection2;
};

class CDDecl2ReplacementActionMap : public std::multimap<CDDeclIndirection, std::shared_ptr<CDDecl2ReplacementAction>> {
public:
	typedef std::multimap<CDDeclIndirection, std::shared_ptr<CDDecl2ReplacementAction>> base_class;
	iterator insert( const std::shared_ptr<CDDecl2ReplacementAction>& cr_shptr ) {
		iterator retval(end());
		if (!cr_shptr) { assert(false); } else {
			value_type val((*cr_shptr).ddecl_indirection_cref(), cr_shptr);
			retval = base_class::insert(val);
		}
		return retval;
	}
	void do_and_dispose_matching_replacements(CState1& state1, const CDDeclIndirection& ddecl_indirection) {
		/* The base class map may be modified during loop iterations. Maybe. */
		auto iter = base_class::find(ddecl_indirection);
		while (base_class::end() != iter) {
			(*((*iter).second)).do_replacement(state1);
			base_class::erase(iter);

			iter = base_class::find(ddecl_indirection);
		}
	}
};

class CDynamicArray2ReplacementAction : public CArray2ReplacementAction {
public:
	using CArray2ReplacementAction::CArray2ReplacementAction;
	virtual ~CDynamicArray2ReplacementAction() {}
};

class CMallocArray2ReplacementAction : public CDynamicArray2ReplacementAction {
public:
	CMallocArray2ReplacementAction(Rewriter &Rewrite, const MatchFinder::MatchResult &MR, const CDDeclIndirection& ddecl_indirection,
			const BinaryOperator* BO, const std::string& bo_replacement_code) :
				CDynamicArray2ReplacementAction(Rewrite, MR, ddecl_indirection), m_BO(BO), m_DD(ddecl_indirection.m_ddecl_cptr),
				m_bo_replacement_code(bo_replacement_code) {}
	virtual ~CMallocArray2ReplacementAction() {}

	virtual void do_replacement(CState1& state1) const;

	const BinaryOperator* m_BO = nullptr;
	//const CallExpr* m_CE = nullptr;
	//const DeclRefExpr* m_DRE = nullptr;
	//const MemberExpr* m_ME = nullptr;
	const DeclaratorDecl* m_DD = nullptr;
	std::string m_bo_replacement_code;
};

class CMallocInitializerArray2ReplacementAction : public CDynamicArray2ReplacementAction {
public:
	CMallocInitializerArray2ReplacementAction(Rewriter &Rewrite, const MatchFinder::MatchResult &MR, const CDDeclIndirection& ddecl_indirection,
			const DeclStmt* DS, const std::string& initializer_info_str) :
				CDynamicArray2ReplacementAction(Rewrite, MR, ddecl_indirection), m_DS(DS), m_DD(ddecl_indirection.m_ddecl_cptr),
				m_initializer_info_str(initializer_info_str) {}
	virtual ~CMallocInitializerArray2ReplacementAction() {}

	virtual void do_replacement(CState1& state1) const;

	const DeclStmt* m_DS = nullptr;
	//const CallExpr* m_CE = nullptr;
	const DeclaratorDecl* m_DD = nullptr;
	std::string m_initializer_info_str;
};

class CFreeDynamicArray2ReplacementAction : public CDynamicArray2ReplacementAction {
public:
	CFreeDynamicArray2ReplacementAction(Rewriter &Rewrite, const MatchFinder::MatchResult &MR, const CDDeclIndirection& ddecl_indirection,
			const CallExpr* CE, const std::string& ce_replacement_code) :
				CDynamicArray2ReplacementAction(Rewrite, MR, ddecl_indirection), m_CE(CE), m_DD(ddecl_indirection.m_ddecl_cptr),
				m_ce_replacement_code(ce_replacement_code) {}
	virtual ~CFreeDynamicArray2ReplacementAction() {}

	virtual void do_replacement(CState1& state1) const;

	const CallExpr* m_CE = nullptr;
	//const DeclRefExpr* m_DRE = nullptr;
	//const MemberExpr* m_ME = nullptr;
	const DeclaratorDecl* m_DD = nullptr;
	std::string m_ce_replacement_code;
};

class CSetArrayPointerToNull2ReplacementAction : public CDynamicArray2ReplacementAction {
public:
	CSetArrayPointerToNull2ReplacementAction(Rewriter &Rewrite, const MatchFinder::MatchResult &MR, const CDDeclIndirection& ddecl_indirection,
			const BinaryOperator* BO, const std::string& bo_replacement_code) :
				CDynamicArray2ReplacementAction(Rewrite, MR, ddecl_indirection), m_BO(BO), m_DD(ddecl_indirection.m_ddecl_cptr),
				m_bo_replacement_code(bo_replacement_code) {}
	virtual ~CSetArrayPointerToNull2ReplacementAction() {}

	virtual void do_replacement(CState1& state1) const;

	const BinaryOperator* m_BO = nullptr;
	//const DeclRefExpr* m_DRE = nullptr;
	//const MemberExpr* m_ME = nullptr;
	const DeclaratorDecl* m_DD = nullptr;
	std::string m_bo_replacement_code;
};

/* This class represents and "enforces" the constraint that the lhs and rhs
 * values of a conditional operator must be the same type. */
class CConditionalOperatorReconciliation2ReplacementAction : public CDynamicArray2ReplacementAction {
public:
	CConditionalOperatorReconciliation2ReplacementAction(Rewriter &Rewrite, const MatchFinder::MatchResult &MR, const CDDeclIndirection& ddecl_indirection,
			const clang::ConditionalOperator* CO, const DeclaratorDecl* lhs_DD, const DeclaratorDecl* rhs_DD, const DeclaratorDecl* var_DD = nullptr) :
				CDynamicArray2ReplacementAction(Rewrite, MR, ddecl_indirection), m_CO(CO), m_lhs_DD(lhs_DD), m_rhs_DD(rhs_DD), m_var_DD(var_DD) {}
	virtual ~CConditionalOperatorReconciliation2ReplacementAction() {}

	virtual void do_replacement(CState1& state1) const;

	const clang::ConditionalOperator* m_CO = nullptr;
	const DeclaratorDecl* m_lhs_DD = nullptr;
	const DeclaratorDecl* m_rhs_DD = nullptr;
	const DeclaratorDecl* m_var_DD = nullptr;
};

class CDynamicArray2ReplacementActionMap : public CDDecl2ReplacementActionMap {
public:
	iterator insert( const std::shared_ptr<CDynamicArray2ReplacementAction>& cr_shptr ) {
		return CDDecl2ReplacementActionMap::insert(static_cast<std::shared_ptr<CDDecl2ReplacementAction> >(cr_shptr));
	}
};

class CArray2ReplacementActionMap : public CDDecl2ReplacementActionMap {
public:
	iterator insert( const std::shared_ptr<CArray2ReplacementAction>& cr_shptr ) {
		return CDDecl2ReplacementActionMap::insert(static_cast<std::shared_ptr<CDDecl2ReplacementAction> >(cr_shptr));
	}
};

class CState1 {
public:
	/* This container holds (potential) actions that are meant to be executed if/when
	 * their corresponding item is determined to be a dynamic array. */
	CDynamicArray2ReplacementActionMap m_dynamic_array2_contingent_replacement_map;

	/* This container holds (potential) actions that are meant to be executed if/when
	 * their corresponding item is determined to be an array (dynamic or otherwise). */
	CArray2ReplacementActionMap m_array2_contingent_replacement_map;

	/* This container holds information about each item's original type and which
	 * type it might be converted to.  */
	CDDeclConversionStateMap m_ddecl_conversion_state_map;
};

class CDeclarationReplacementCodeItem {
public:
	std::string m_replacement_code;
	std::string m_action_species;
};
static CDeclarationReplacementCodeItem generate_declaration_replacement_code(const DeclaratorDecl* DD, Rewriter &Rewrite, CDDeclConversionStateMap& ddecl_conversion_state_map, std::string options_str = "") {
	CDeclarationReplacementCodeItem retval;

	if (!DD) {
		return retval;
	}
	auto decl_source_range = nice_source_range(DD->getSourceRange(), Rewrite);
	if (!(decl_source_range.isValid())) {
		return retval;
	}

	auto res1 = ddecl_conversion_state_map.insert(*DD);
	auto ddcs_map_iter = res1.first;
	auto& ddcs_ref = (*ddcs_map_iter).second;

	QualType QT = DD->getType();
	const clang::Type* TP = QT.getTypePtr();
	auto qtype_str = QT.getAsString();
	auto direct_qtype_str = clang::QualType::getAsString(ddcs_ref.m_direct_qtype.split());
	if ("_Bool" == direct_qtype_str) {
		direct_qtype_str = "bool";
	} else if ("const _Bool" == direct_qtype_str) {
		direct_qtype_str = "const bool";
	}
	auto direct_TP = ddcs_ref.m_direct_qtype.getTypePtr();
	if (!direct_TP) {
		return retval;
	}
	bool direct_type_is_function_type = direct_TP->isFunctionType();

	std::string variable_name = DD->getNameAsString();
	std::string identifier_name_str;
	auto pIdentifier = DD->getIdentifier();
	if (pIdentifier) {
		identifier_name_str = pIdentifier->getName();
	}
	if ("" == variable_name) {
		int q = 7;
	}

	if (!(ddcs_ref.m_original_source_text_has_been_noted)) {
		ddcs_ref.m_original_source_text_str = Rewrite.getRewrittenText(decl_source_range);
		ddcs_ref.m_original_source_text_has_been_noted = true;
	}

	clang::StorageClass storage_class = clang::StorageClass::SC_None;
	bool is_extern = false;
	clang::StorageDuration storage_duration = clang::StorageDuration::SD_Automatic;
	bool has_dynamic_storage_duration = false;
	bool is_a_temporary = false;
	bool is_static = false;
	bool is_a_function_parameter = false;
	bool is_member = false;
	bool is_vardecl = false;
	std::string initialization_expr_str;
	bool is_function = DD->isFunctionOrFunctionTemplate();

	auto VD = dynamic_cast<const clang::VarDecl *>(DD);
	if (VD) {
		is_vardecl = true;
		storage_class = VD->getStorageClass();
		is_extern = (clang::StorageClass::SC_Extern == storage_class);
		storage_duration = VD->getStorageDuration();
		has_dynamic_storage_duration = (clang::StorageDuration::SD_Dynamic == storage_duration);
		is_a_temporary = (clang::StorageDuration::SD_FullExpression == storage_duration);
		//is_static = (clang::StorageDuration::SD_Static == storage_duration);
		is_static = (clang::StorageClass::SC_Static == storage_class);
		if ((clang::StorageDuration::SD_Static == storage_duration) && (!is_static)) {
			int q = 5;
		}
		is_a_function_parameter = (VD->isLocalVarDeclOrParm() && (!VD->isLocalVarDecl()));

		if (ddcs_ref.m_original_initialization_has_been_noted) {
			initialization_expr_str = ddcs_ref.m_original_initialization_expr_str;
			if ("" != ddcs_ref.m_original_initialization_expr_str) {
				int q = 5;
			}
		} else {
			if (VD->hasInit()) {
				auto pInitExpr = VD->getInit();
				if (pInitExpr) {
					auto init_expr_source_range = nice_source_range(pInitExpr->getSourceRange(), Rewrite);
					if (init_expr_source_range.isValid()) {
						initialization_expr_str = Rewrite.getRewrittenText(init_expr_source_range);
						if (variable_name == initialization_expr_str) {
							/* We encountered a weird bug where the initialization expression sometimes
							 * was indicated as being present and the source range set to the variable name
							 * when actually no initialization expression was present in the original source. */
							initialization_expr_str = "";
						}
					} else {
						int q = 3;
					}
				} else {
					int q = 3;
				}
				ddcs_ref.m_original_initialization_expr_str = initialization_expr_str;
			}
			ddcs_ref.m_original_initialization_has_been_noted = true;
		}
	} else {
		auto FD = dynamic_cast<const clang::FieldDecl*>(DD);
		if (FD) {
			is_member = true;

			if (ddcs_ref.m_original_initialization_has_been_noted) {
				initialization_expr_str = ddcs_ref.m_original_initialization_expr_str;
				if ("" != ddcs_ref.m_original_initialization_expr_str) {
					int q = 5;
				}
			} else {
				if (FD->hasInClassInitializer()) {
					auto pInitExpr = FD->getInClassInitializer();
					if (pInitExpr) {
						auto init_expr_source_range = nice_source_range(pInitExpr->getSourceRange(), Rewrite);
						if (init_expr_source_range.isValid()) {
							initialization_expr_str = Rewrite.getRewrittenText(init_expr_source_range);
							if (variable_name == initialization_expr_str) {
								/* We encountered a weird bug where the initialization expression sometimes
								 * was indicated as being present and the source range set to the variable name
								 * when actually no initialization expression was present in the original source. */
								initialization_expr_str = "";
							}
						} else {
							int q = 3;
						}
					} else {
						int q = 3;
					}
					ddcs_ref.m_original_initialization_expr_str = initialization_expr_str;
				}
				ddcs_ref.m_original_initialization_has_been_noted = true;
			}
		}
	}
	ddcs_ref.m_original_initialization_has_been_noted = true;

	bool changed_from_original = false;
	std::string replacement_code;
	std::string prefix_str;
	std::string suffix_str;

	if (true) {
		for (size_t i = 0; i < ddcs_ref.m_indirection_state_stack.size(); i += 1) {
			bool is_char_star = false;
			bool is_function_pointer = false;
			bool is_last_indirection = (ddcs_ref.m_indirection_state_stack.size() == (i+1));
			if (is_last_indirection && (("char" == direct_qtype_str) || ("const char" == direct_qtype_str))) {
				is_char_star = true;
			} else if (is_last_indirection && direct_type_is_function_type) {
				is_function_pointer = true;
			}
			if ("inferred array" == ddcs_ref.m_indirection_state_stack[i].m_current) {
				if (is_char_star) {
					/* We're assuming this is a null terminated string. We'll just leave it as a
					 * char* for now. At some point we'll replace it with an mse::string or whatever. */
					//prefix_str = prefix_str + "";
					suffix_str = "* " + suffix_str;
					retval.m_action_species = "char*";
				} else {
					prefix_str = prefix_str + "mse::TNullableAnyRandomAccessIterator<";
					suffix_str = "> " + suffix_str;
					retval.m_action_species = "native pointer to TNullableAnyRandomAccessIterator";
				}
			} else if ("dynamic array" == ddcs_ref.m_indirection_state_stack[i].m_current) {
				if (is_char_star) {
					/* We're assuming this is a null terminated string. We'll just leave it as a
					 * char* for now. At some point we'll replace it with an mse::string or whatever. */
					//prefix_str = prefix_str + "";
					suffix_str = "* " + suffix_str;
					retval.m_action_species = "char*";
				} else {
					prefix_str = prefix_str + "mse::TIPointerWithBundledVector<";
					if (is_a_function_parameter) {
						suffix_str = "> " + suffix_str;
						retval.m_action_species = "native pointer parameter to TIPointerWithBundledVector";
					} else {
						suffix_str = "> " + suffix_str;
						retval.m_action_species = "native pointer to TIPointerWithBundledVector";
					}
				}
			} else if ("native array" == ddcs_ref.m_indirection_state_stack[i].m_current) {
				std::string size_text;
				if (TP->isVariableArrayType()) {
					auto VATP = llvm::cast<const clang::VariableArrayType>(TP);
					if (!VATP) {
						assert(false);
					} else {
						auto size_expr = VATP->getSizeExpr();
						auto SR = nice_source_range(size_expr->getSourceRange(), Rewrite);
						size_text = Rewrite.getRewrittenText(SR);
					}
				} else if (TP->isConstantArrayType()) {
					auto CATP = llvm::cast<const clang::ConstantArrayType>(TP);
					if (!CATP) {
						assert(false);
					} else {
						auto array_size = CATP->getSize();
						size_text = array_size.toString(10, false);/*check this*/

						if (false) {
							auto DDSR = nice_source_range(DD->getSourceRange(), Rewrite);
							std::string array_size_expression_text;
							std::string source_text;
							if (DDSR.isValid()) {
								source_text = Rewrite.getRewrittenText(DDSR);

								auto left_bracket_pos = source_text.find('[');
								auto right_bracket_pos = source_text.find(']');
								if ((std::string::npos != left_bracket_pos) && (std::string::npos != right_bracket_pos)
										&& (left_bracket_pos + 1 < right_bracket_pos)) {
									auto array_size_expression_text = source_text.substr(left_bracket_pos + 1, right_bracket_pos - (left_bracket_pos + 1));
									int q = 3;
								} else {
									int q = 7;
								}
							} else {
								int q = 5;
							}
						}
					}
				}

				if (is_char_star) {
					/* We're assuming this is a null terminated string. We'll just leave it as a
					 * char[] for now. At some point we'll replace it with an mse::string or whatever. */
					//prefix_str = prefix_str + "";
					suffix_str = "[" + size_text + "]" + suffix_str;
				} else {
					if (is_a_function_parameter) {
						prefix_str = prefix_str + "mse::TNullableAnyRandomAccessIterator<";
						suffix_str = ", " + size_text + "> " + suffix_str;
						retval.m_action_species = "native array parameter to TNullableAnyRandomAccessIterator";
					} else {
						prefix_str = prefix_str + "mse::TIteratorWithBundledArray<";
						suffix_str = ", " + size_text + "> " + suffix_str;
						retval.m_action_species = "native array to TIteratorWithBundledArray";
					}
				}
			} else if ("native pointer" == ddcs_ref.m_indirection_state_stack[i].m_current) {
				if (is_char_star) {
					/* We're assuming this is a null terminated string. We'll just leave it as a
					 * char* for now. At some point we'll replace it with an mse::string or whatever. */
					//prefix_str = prefix_str + "";
					suffix_str = "* " + suffix_str;
					retval.m_action_species = "char*";
				} else if (is_function_pointer) {
					prefix_str = prefix_str + "std::function<";
					suffix_str = "> " + suffix_str;
					retval.m_action_species = "function pointer to std::function";
				} else {
					if (false/*for now*/) {
						prefix_str = prefix_str + "mse::TAnyPointer<";
						suffix_str = "> " + suffix_str;
						retval.m_action_species = "native pointer to TAnyPointer";
					} else {
						//prefix_str = prefix_str + "";
						suffix_str = "* " + suffix_str;
						retval.m_action_species = "native pointer";
					}
				}
			} else if ("malloc target" == ddcs_ref.m_indirection_state_stack[i].m_current) {
				/* We'll just leaving it as a native pointer for now. Ultimately, this won't be the case. */
				//prefix_str = prefix_str + "";
				suffix_str = "* " + suffix_str;
				retval.m_action_species = "malloc target";
			}
		}
	}

	bool discard_initializer_option_flag = (std::string::npos != options_str.find("[discard-initializer]"));
	std::string initializer_append_str;
	if (!discard_initializer_option_flag) {
		initializer_append_str = ddcs_ref.m_initializer_info_str;
		if (("" == initializer_append_str) && ("" != initialization_expr_str)) {
			initializer_append_str = " = " + initialization_expr_str;
		}
	}

	if (("" != prefix_str) || ("" != suffix_str)) {
		changed_from_original = true;
	} else if (("" != ddcs_ref.m_initializer_info_str) ||
			(discard_initializer_option_flag)) {
		changed_from_original = true;
	} else if (2 <= IndividualDeclaratorDecls(DD, Rewrite).size()) {
		/* There is more than one declaration in the declaration statement. We split
		 * them so that each has their own separate declaration statement. This counts
		 * as a change from the original source code. */
		changed_from_original = true;
	}

	if (changed_from_original) {
		if (is_extern) {
			if ("" == ddcs_ref.m_original_initialization_expr_str) {
				replacement_code += "extern ";
			}
		} else if (is_static) {
			replacement_code += "static ";
		}
		replacement_code += prefix_str + direct_qtype_str + suffix_str;
		replacement_code += " ";
		replacement_code += variable_name;

		replacement_code += initializer_append_str;
	} else {
		replacement_code = ddcs_ref.m_original_source_text_str;
	}

	retval.m_replacement_code = replacement_code;
	return retval;
}

static void update_declaration(const DeclaratorDecl& ddecl, Rewriter &Rewrite, CState1& state1, std::string options_str = "") {
	const DeclaratorDecl* DD = &ddecl;
	auto SR = nice_source_range(DD->getSourceRange(), Rewrite);

	QualType QT = DD->getType();
	const clang::Type* TP = QT.getTypePtr();
	auto qtype_str = QT.getAsString();

	std::string source_text;
	if (SR.isValid()) {
		source_text = Rewrite.getRewrittenText(SR);
		if ("" == source_text) {
			return;
		}
	} else {
		return;
	}

	std::string variable_name = DD->getNameAsString();

	if (("" == variable_name) || (!TP)) {
		return;
	}

	if ((TP->isFunctionType()) || (false)) {
		/* We don't handle function declarations yet. */
		return;
	}

	/* There may be multiple declarations in the same declaration statement. Replacing
	 * one of them requires replacing all of them together. */
	auto ddecls = IndividualDeclaratorDecls(DD, Rewrite);
	if ((1 <= ddecls.size())/* && (ddecls.back() == DD)*/) {
		if (2 <= ddecls.size()) {
			int q = 5;
		}
		std::vector<std::string> action_species_list;
		std::string replacement_code;
		for (const auto& ddecl_cref : ddecls) {
			auto res = generate_declaration_replacement_code(ddecl_cref, Rewrite, state1.m_ddecl_conversion_state_map, options_str);
			action_species_list.push_back(res.m_action_species);
			replacement_code += res.m_replacement_code;
			replacement_code += "; \n";
		}
		if (replacement_code.size() >= 3) {
			replacement_code = replacement_code.substr(0, replacement_code.size() - 3);
		}

		/* (Only) the source range of the last individual declaration in the declaration statement
		 * should encompass the whole statement. */
		auto last_ddecl = ddecls.back();
		auto last_decl_source_range = nice_source_range(last_ddecl->getSourceRange(), Rewrite);

		std::string last_decl_source_text;
		if (last_decl_source_range.isValid()) {
			last_decl_source_text = Rewrite.getRewrittenText(last_decl_source_range);
			if ("" == last_decl_source_text) {
				return;
			}
		} else {
			return;
		}

		if (ConvertToSCPP && last_decl_source_range.isValid() && (3 <= replacement_code.size())) {
			auto res = Rewrite.ReplaceText(last_decl_source_range, replacement_code);
		} else {
			int q = 7;
		}
	} else {
		int q = 7;
	}
}

void CMallocArray2ReplacementAction::do_replacement(CState1& state1) const {
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

		if (ConvertToSCPP && decl_source_range.isValid() && (BOSR.isValid())) {

			update_declaration(*DD, Rewrite, state1);

			//state1.m_array2_contingent_replacement_map.do_and_dispose_matching_replacements(state1, (*this).ddecl_indirection_cref());
			//state1.m_dynamic_array2_contingent_replacement_map.do_and_dispose_matching_replacements(state1, (*this).ddecl_indirection_cref());

			auto res2 = Rewrite.ReplaceText(BOSR, m_bo_replacement_code);
			int q = 3;
		} else {
			int q = 7;
		}
	}
}

void CMallocInitializerArray2ReplacementAction::do_replacement(CState1& state1) const {
	Rewriter &Rewrite = m_Rewrite;
	const MatchFinder::MatchResult &MR = m_MR;
	const DeclStmt* DS = m_DS;
	const DeclaratorDecl* DD = m_DD;

	if ((DS != nullptr) && (DD != nullptr))
	{
		auto decl_source_range = nice_source_range(DD->getSourceRange(), Rewrite);
		auto decl_source_location_str = decl_source_range.getBegin().printToString(*MR.SourceManager);
		std::string decl_source_text;
		if (decl_source_range.isValid()) {
			decl_source_text = Rewrite.getRewrittenText(decl_source_range);
		} else {
			return;
		}

		auto res1 = state1.m_ddecl_conversion_state_map.insert(*DD);
		auto ddcs_map_iter = res1.first;
		auto& ddcs_ref = (*ddcs_map_iter).second;

		ddcs_ref.m_initializer_info_str = m_initializer_info_str;

		if (ConvertToSCPP && decl_source_range.isValid()) {
			update_declaration(*DD, Rewrite, state1);

			//state1.m_array2_contingent_replacement_map.do_and_dispose_matching_replacements(state1, (*this).ddecl_indirection_cref());
			//state1.m_dynamic_array2_contingent_replacement_map.do_and_dispose_matching_replacements(state1, (*this).ddecl_indirection_cref());

			int q = 3;
		} else {
			int q = 7;
		}
	}
}

void CFreeDynamicArray2ReplacementAction::do_replacement(CState1& state1) const {
	Rewriter &Rewrite = m_Rewrite;
	const MatchFinder::MatchResult &MR = m_MR;
	const CallExpr* CE = m_CE;
	const DeclaratorDecl* DD = m_DD;

	if ((CE != nullptr) && (DD != nullptr))
	{
		auto CESR = nice_source_range(CE->getSourceRange(), Rewrite);
		auto decl_source_range = nice_source_range(DD->getSourceRange(), Rewrite);
		auto decl_source_location_str = decl_source_range.getBegin().printToString(*MR.SourceManager);
		std::string decl_source_text;
		if (decl_source_range.isValid()) {
			decl_source_text = Rewrite.getRewrittenText(decl_source_range);
		} else {
			return;
		}

		if (ConvertToSCPP && decl_source_range.isValid() && (CESR.isValid())) {

			update_declaration(*DD, Rewrite, state1);

			//state1.m_array2_contingent_replacement_map.do_and_dispose_matching_replacements(state1, (*this).ddecl_indirection_cref());
			//state1.m_dynamic_array2_contingent_replacement_map.do_and_dispose_matching_replacements(state1, (*this).ddecl_indirection_cref());

			auto res2 = Rewrite.ReplaceText(CESR, m_ce_replacement_code);
			int q = 3;
		} else {
			int q = 7;
		}
	}
}

void CSetArrayPointerToNull2ReplacementAction::do_replacement(CState1& state1) const {
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

		if (ConvertToSCPP && decl_source_range.isValid() && (BOSR.isValid())) {

			update_declaration(*DD, Rewrite, state1);

			auto res2 = Rewrite.ReplaceText(BOSR, m_bo_replacement_code);
			int q = 3;
		} else {
			int q = 7;
		}
	}
}

void CMemsetArray2ReplacementAction::do_replacement(CState1& state1) const {
	Rewriter &Rewrite = m_Rewrite;
	const MatchFinder::MatchResult &MR = m_MR;
	const CallExpr* CE = m_CE;
	const DeclaratorDecl* DD = m_DD;

	if ((CE != nullptr) && (DD != nullptr))
	{
		auto CESR = nice_source_range(CE->getSourceRange(), Rewrite);
		auto decl_source_range = nice_source_range(DD->getSourceRange(), Rewrite);
		auto decl_source_location_str = decl_source_range.getBegin().printToString(*MR.SourceManager);
		std::string decl_source_text;
		if (decl_source_range.isValid()) {
			decl_source_text = Rewrite.getRewrittenText(decl_source_range);
		} else {
			return;
		}

		if (ConvertToSCPP && decl_source_range.isValid() && (CESR.isValid())) {

			update_declaration(*DD, Rewrite, state1);

			auto res2 = Rewrite.ReplaceText(CESR, m_ce_replacement_code);
			int q = 3;
		} else {
			int q = 7;
		}
	}
}

void CMemcpyArray2ReplacementAction::do_replacement(CState1& state1) const {
	Rewriter &Rewrite = m_Rewrite;
	const MatchFinder::MatchResult &MR = m_MR;
	const CallExpr* CE = m_CE;
	const DeclaratorDecl* DD = m_DD;

	if ((CE != nullptr) && (DD != nullptr))
	{
		auto CESR = nice_source_range(CE->getSourceRange(), Rewrite);
		auto decl_source_range = nice_source_range(DD->getSourceRange(), Rewrite);
		auto decl_source_location_str = decl_source_range.getBegin().printToString(*MR.SourceManager);
		std::string decl_source_text;
		if (decl_source_range.isValid()) {
			decl_source_text = Rewrite.getRewrittenText(decl_source_range);
		} else {
			return;
		}

		if (ConvertToSCPP && decl_source_range.isValid() && (CESR.isValid())) {

			update_declaration(*DD, Rewrite, state1);

			auto res2 = Rewrite.ReplaceText(CESR, m_ce_replacement_code);
			int q = 3;
		} else {
			int q = 7;
		}
	}
}

void CAssignedFromArray2ReplacementAction::do_replacement(CState1& state1) const {
	Rewriter &Rewrite = m_Rewrite;
	const MatchFinder::MatchResult &MR = m_MR;

	auto res1 = state1.m_ddecl_conversion_state_map.insert(*(m_ddecl_indirection2.m_ddecl_cptr));
	auto ddcs_map_iter = res1.first;
	auto& ddcs_ref = (*ddcs_map_iter).second;
	bool update_declaration_flag = res1.second;

	if (ddcs_ref.m_indirection_state_stack.size() >= m_ddecl_indirection2.m_indirection_level) {
		if ("native pointer" == ddcs_ref.m_indirection_state_stack[m_ddecl_indirection2.m_indirection_level].m_current) {
			ddcs_ref.m_indirection_state_stack[m_ddecl_indirection2.m_indirection_level].m_current = "inferred array";
			update_declaration_flag = true;
			state1.m_array2_contingent_replacement_map.do_and_dispose_matching_replacements(state1, m_ddecl_indirection2);
		} else if ("malloc target" == ddcs_ref.m_indirection_state_stack[m_ddecl_indirection2.m_indirection_level].m_current) {
			ddcs_ref.m_indirection_state_stack[m_ddecl_indirection2.m_indirection_level].m_current = "dynamic array";
			update_declaration_flag = true;
			state1.m_array2_contingent_replacement_map.do_and_dispose_matching_replacements(state1, m_ddecl_indirection2);
			state1.m_dynamic_array2_contingent_replacement_map.do_and_dispose_matching_replacements(state1, m_ddecl_indirection2);
		} else {
			int q = 3;
		}
	} else {
		int q = 7;
	}

	if (update_declaration_flag) {
		update_declaration(*(m_ddecl_indirection2.m_ddecl_cptr), Rewrite, state1);
	}
}

void CConditionalOperatorReconciliation2ReplacementAction::do_replacement(CState1& state1) const {
	const clang::ConditionalOperator* CO = m_CO;
	const Expr* COND = nullptr;
	const Expr* LHS = nullptr;
	const Expr* RHS = nullptr;
	if (CO) {
		COND = CO->getCond();
		LHS = CO->getLHS();
		RHS = CO->getRHS();
	}
	const DeclaratorDecl* lhs_DD = m_lhs_DD;
	const DeclaratorDecl* rhs_DD = m_rhs_DD;
	if ((COND != nullptr) && (LHS != nullptr) && (RHS != nullptr) && (lhs_DD != nullptr) && (rhs_DD != nullptr)) {
		bool lhs_is_array = false;
		bool lhs_is_dynamic_array = false;
		bool lhs_is_native_array = false;

		auto COSR = nice_source_range(CO->getSourceRange(), (*this).m_Rewrite);
		auto cond_SR = nice_source_range(COND->getSourceRange(), (*this).m_Rewrite);
		auto lhs_SR = nice_source_range(LHS->getSourceRange(), (*this).m_Rewrite);
		auto rhs_SR = nice_source_range(RHS->getSourceRange(), (*this).m_Rewrite);
		if ((COSR.isValid()) && (cond_SR.isValid()) && (lhs_SR.isValid()) && (rhs_SR.isValid())) {
			auto res1 = state1.m_ddecl_conversion_state_map.insert(*lhs_DD);
			auto ddcs_map_iter = res1.first;
			auto& ddcs_ref = (*ddcs_map_iter).second;
			if (1 <= ddcs_ref.m_indirection_state_stack.size()) {
				if ("dynamic array" == ddcs_ref.m_indirection_state_stack[0].m_current) {
					lhs_is_dynamic_array = true;
					lhs_is_array = true;
				} else if ("native array" == ddcs_ref.m_indirection_state_stack[0].m_current) {
					lhs_is_native_array = true;
					lhs_is_array = true;
				} else if ("inferred array" == ddcs_ref.m_indirection_state_stack[0].m_current) {
					lhs_is_array = true;
				}
			} else {
				int q = 3;
			}
		}

		bool rhs_is_array = false;
		bool rhs_is_dynamic_array = false;
		bool rhs_is_native_array = false;
		{
			auto res1 = state1.m_ddecl_conversion_state_map.insert(*rhs_DD);
			auto ddcs_map_iter = res1.first;
			auto& ddcs_ref = (*ddcs_map_iter).second;
			if (1 <= ddcs_ref.m_indirection_state_stack.size()) {
				if ("dynamic array" == ddcs_ref.m_indirection_state_stack[0].m_current) {
					rhs_is_dynamic_array = true;
					rhs_is_array = true;
				} else if ("native array" == ddcs_ref.m_indirection_state_stack[0].m_current) {
					rhs_is_native_array = true;
					rhs_is_array = true;
				} else if ("inferred array" == ddcs_ref.m_indirection_state_stack[0].m_current) {
					rhs_is_array = true;
				}
			} else {
				int q = 3;
			}
		}
		if (lhs_is_array && rhs_is_array) {
			if (m_var_DD) {
				update_declaration(*m_var_DD, (*this).m_Rewrite, state1);
			}

			std::string CO_prior_text = (*this).m_Rewrite.getRewrittenText(COSR);
			std::string CO_replacement_text;

			std::string cond_prior_text = (*this).m_Rewrite.getRewrittenText(cond_SR);
			std::string cond_replacement_text = cond_prior_text;

			std::string lhs_prior_text = (*this).m_Rewrite.getRewrittenText(lhs_SR);
			std::string lhs_replacement_text = lhs_prior_text;

			std::string rhs_prior_text = (*this).m_Rewrite.getRewrittenText(rhs_SR);
			std::string rhs_replacement_text = rhs_prior_text;
			static const std::string ara_iter_prefix = "mse::TAnyRandomAccessIterator<";

			bool lhs_needs_to_be_wrapped = false;
			if ((lhs_is_dynamic_array && (!rhs_is_dynamic_array)) || (lhs_is_native_array && (!rhs_is_native_array))) {
				lhs_needs_to_be_wrapped = true;
			}
			if (lhs_needs_to_be_wrapped) {
				if (!string_begins_with(lhs_prior_text, ara_iter_prefix)) {
					std::string lhs_base_type_str = lhs_DD->getType().getAsString();
					std::string lhs_replacement_text = ara_iter_prefix + lhs_base_type_str + " >(" + lhs_prior_text + ")";
					if (ConvertToSCPP) {
						auto res2 = (*this).m_Rewrite.ReplaceText(lhs_SR, lhs_replacement_text);

						CO_replacement_text = (*this).m_Rewrite.getRewrittenText(COSR);
						int q = 3;
					}
				}
			}

			bool rhs_needs_to_be_wrapped = false;
			if ((rhs_is_dynamic_array && (!lhs_is_dynamic_array)) || (rhs_is_native_array && (!lhs_is_native_array))) {
				rhs_needs_to_be_wrapped = true;
			}
			if (rhs_needs_to_be_wrapped) {
				if (!string_begins_with(rhs_prior_text, ara_iter_prefix)) {
					std::string rhs_base_type_str = rhs_DD->getType().getAsString();
					std::string rhs_replacement_text = ara_iter_prefix + rhs_base_type_str + " >(" + rhs_prior_text + ")";
					if (ConvertToSCPP) {
						auto res2 = (*this).m_Rewrite.ReplaceText(rhs_SR, rhs_replacement_text);

						auto possibly_truncated_CO_replacement_text = (*this).m_Rewrite.getRewrittenText(COSR);
						std::string cond_text = (*this).m_Rewrite.getRewrittenText(cond_SR);
						std::string CO_replacement_text2 = cond_text + " ? " + lhs_replacement_text + " : " + rhs_replacement_text;

						CO_replacement_text = possibly_truncated_CO_replacement_text;
						int q = 3;
					}
				}
			}

			if (m_var_DD && ("" != CO_replacement_text)) {
				auto res1 = state1.m_ddecl_conversion_state_map.insert(*m_var_DD);
				auto ddcs_map_iter = res1.first;
				auto& ddcs_ref = (*ddcs_map_iter).second;

				ddcs_ref.m_initializer_info_str = " = " + CO_replacement_text;
			}

		}
	}
}


struct CArrayInferenceInfo {
	bool update_declaration_flag = false;
	bool has_been_determined_to_be_an_array = false;
	size_t indirection_level = 0;
	const DeclaratorDecl* ddecl_cptr = nullptr;
	const clang::Expr* declaration_expr_cptr = nullptr;
};

CArrayInferenceInfo infer_array_type_info_from_stmt_indirection_stack(CDDeclConversionState& ddcs_ref,
		const std::vector<std::string>& stmt_indirection_stack, CState1& state1_ref) {
	CArrayInferenceInfo retval;
	auto DD = ddcs_ref.m_ddecl_cptr;
	if (!DD) { assert(false); return retval; }
	for (size_t i = 0; ((i < ddcs_ref.m_indirection_state_stack.size())
			&& (i < stmt_indirection_stack.size())); i += 1) {
		if (("" == stmt_indirection_stack[i])) {
			/* We're using the empty string as a generic state for the "terminal level of indirection"
			 * when we don't want to bother specifying a specific state. */
			retval.indirection_level = i;
		} else if ("native pointer" == ddcs_ref.m_indirection_state_stack[i].m_current) {
			if (("ArraySubscriptExpr" == stmt_indirection_stack[i])
					|| ("pointer arithmetic" == stmt_indirection_stack[i])) {
				ddcs_ref.m_indirection_state_stack[i].m_current = "inferred array";
				retval.update_declaration_flag = true;
				state1_ref.m_array2_contingent_replacement_map.do_and_dispose_matching_replacements(state1_ref, CDDeclIndirection(*DD, i));
			} else if (("malloc target" == stmt_indirection_stack[i])) {
				ddcs_ref.m_indirection_state_stack[i].m_current = "malloc target";
				retval.indirection_level = i;
			} else if (("set to null" == stmt_indirection_stack[i]) ||
					("memset/cpy target" == stmt_indirection_stack[i])) {
				retval.indirection_level = i;
			}
		} else if ("malloc target" == ddcs_ref.m_indirection_state_stack[i].m_current) {
			if (("ArraySubscriptExpr" == stmt_indirection_stack[i])
					|| ("pointer arithmetic" == stmt_indirection_stack[i])) {
				ddcs_ref.m_indirection_state_stack[i].m_current = "dynamic array";
				retval.update_declaration_flag = true;
				state1_ref.m_array2_contingent_replacement_map.do_and_dispose_matching_replacements(state1_ref, CDDeclIndirection(*DD, i));
				state1_ref.m_dynamic_array2_contingent_replacement_map.do_and_dispose_matching_replacements(state1_ref, CDDeclIndirection(*DD, i));
			}
		} else if ("inferred array" == ddcs_ref.m_indirection_state_stack[i].m_current) {
			if (("malloc target" == stmt_indirection_stack[i]) ||
					("set to null" == stmt_indirection_stack[i])) {
				ddcs_ref.m_indirection_state_stack[i].m_current = "dynamic array";
				retval.update_declaration_flag = true;
				retval.has_been_determined_to_be_an_array = true;
				retval.indirection_level = i;
				state1_ref.m_dynamic_array2_contingent_replacement_map.do_and_dispose_matching_replacements(state1_ref, CDDeclIndirection(*DD, i));
			} else if (("memset/cpy target" == stmt_indirection_stack[i])) {
				retval.has_been_determined_to_be_an_array = true;
				retval.indirection_level = i;
			}
		} else if ("dynamic array" == ddcs_ref.m_indirection_state_stack[i].m_current) {
			if (("malloc target" == stmt_indirection_stack[i]) ||
					("set to null" == stmt_indirection_stack[i])) {
				retval.has_been_determined_to_be_an_array = true;
				retval.indirection_level = i;
			}
		}
	}
	return retval;
}

/* We are trying to determine, for each and every pointer, whether or not it is being used as an
 * array iterator. So this function takes an expression using a declared (pointer) variable
 * and notes when the (pointer) variable, or any dereference of the
 * (pointer) variable, is being used as an array iterator. So for example, given a declaration,
 * say, "int*** ptr1;" and an expression, say, "(*(ptr1[3]))[5]", it will note the two levels
 * of dereference/"indirection" that are being used as array iterators/pointers. Upon determining
 * that a pointer (or a dereference/indirection of the pointer) is being used as an array iterator,
 * the function will execute any queued up actions that were contingent on such a determination. */
CArrayInferenceInfo infer_array_type_info_from_stmt(const clang::Stmt& stmt_cref, const std::string& stmt_array_info_str,
		CState1& state1_ref, const DeclaratorDecl* DD = nullptr) {
	CArrayInferenceInfo retval;

	std::vector<std::string> stmt_indirection_stack;
	const clang::Expr* expr2 = populateStmtIndirectionStack(stmt_indirection_stack, stmt_cref);
	std::reverse(stmt_indirection_stack.begin(), stmt_indirection_stack.end());
	stmt_indirection_stack.push_back(stmt_array_info_str);
	if (expr2) {
		std::string expr2_stmt_class_name;
		expr2_stmt_class_name = expr2->getStmtClassName();
		const DeclaratorDecl* expr2_DD = nullptr;
		if (clang::Stmt::StmtClass::DeclRefExprClass == expr2->getStmtClass()) {
			auto expr2_DRE = llvm::cast<const clang::DeclRefExpr>(expr2);
			if (expr2_DRE) {
				auto expr2_decl = expr2_DRE->getDecl();
				expr2_DD = dynamic_cast<const DeclaratorDecl*>(expr2_decl);
			} else { assert(false); }
		} else if (clang::Stmt::StmtClass::MemberExprClass == expr2->getStmtClass()) {
			auto expr2_ME = llvm::cast<const clang::MemberExpr>(expr2);
			if (expr2_ME) {
				auto expr2_FD = dynamic_cast<const clang::FieldDecl*>(expr2_ME->getMemberDecl());
				if (expr2_FD) {
					expr2_DD = expr2_FD;
				} else { assert(false); }
			} else { assert(false); }
		}
		if (expr2_DD) {
			auto expr2_QT = expr2_DD->getType();
			auto expr2_type_str = expr2_QT.getAsString();
			std::string expr2_variable_name = expr2_DD->getNameAsString();

			if (nullptr == DD) {
				DD = expr2_DD;
			}
			auto res1 = state1_ref.m_ddecl_conversion_state_map.insert(*DD);
			auto ddcs_map_iter = res1.first;
			auto& ddcs_ref = (*ddcs_map_iter).second;
			bool update_declaration_flag = res1.second;

			auto QT = (*DD).getType();
			std::string variable_name = (*DD).getNameAsString();

			if ((expr2_QT == QT) && (expr2_variable_name == variable_name)) {
				retval = infer_array_type_info_from_stmt_indirection_stack(ddcs_ref, stmt_indirection_stack, state1_ref);
			}
		}
		retval.ddecl_cptr = expr2_DD;
	}
	retval.declaration_expr_cptr = expr2;

	return retval;
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

class MCSSSVarDecl2 : public MatchFinder::MatchCallback
{
public:
	MCSSSVarDecl2 (Rewriter &Rewrite, CState1& state1) :
		Rewrite(Rewrite), m_state1(state1) {}

	virtual void run(const MatchFinder::MatchResult &MR)
	{
		const DeclaratorDecl* DD = MR.Nodes.getNodeAs<clang::DeclaratorDecl>("mcsssvardecl");
		if ((DD != nullptr))
		{
			auto SR = nice_source_range(DD->getSourceRange(), Rewrite);
			auto decl_source_range = SR;
			SourceLocation SL = SR.getBegin();
			SourceLocation SLE = SR.getEnd();

			QualType QT = DD->getType();
			const clang::Type* TP = QT.getTypePtr();
			auto qtype_str = QT.getAsString();

			ASTContext *const ASTC = MR.Context;
			FullSourceLoc FSL = ASTC->getFullLoc(SL);

			SourceManager &SM = ASTC->getSourceManager();

			auto source_location_str = SL.printToString(*MR.SourceManager);
			std::string source_text;
			if (SR.isValid()) {
				source_text = Rewrite.getRewrittenText(SR);
			} else {
				return;
			}

			if (filtered_out_by_location(MR, SL)) {
				return void();
			}

			std::string variable_name = DD->getNameAsString();

			auto qualified_name = DD->getQualifiedNameAsString();
			static const std::string mse_namespace_str1 = "mse::";
			static const std::string mse_namespace_str2 = "::mse::";
			if ((0 == qualified_name.compare(0, mse_namespace_str1.size(), mse_namespace_str1))
					|| (0 == qualified_name.compare(0, mse_namespace_str2.size(), mse_namespace_str2))) {
				int q = 5;
				//return;
			}

			auto res1 = (*this).m_state1.m_ddecl_conversion_state_map.insert(*DD);
			auto ddcs_map_iter = res1.first;
			auto& ddcs_ref = (*ddcs_map_iter).second;
			//bool update_declaration_flag = res1.second;

			for (size_t i = 0; (i < ddcs_ref.m_indirection_state_stack.size()); i += 1) {
				if ("native array" == ddcs_ref.m_indirection_state_stack[i].m_current) {
					m_state1.m_array2_contingent_replacement_map.do_and_dispose_matching_replacements(m_state1, CDDeclIndirection(*DD, i));
				}
			}

			update_declaration(*DD, Rewrite, m_state1);
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

class MCSSSPointerArithmetic2 : public MatchFinder::MatchCallback
{
public:
	MCSSSPointerArithmetic2 (Rewriter &Rewrite, CState1& state1)
: Rewrite(Rewrite), m_state1(state1) {}

	virtual void run(const MatchFinder::MatchResult &MR)
	{
		const DeclRefExpr* DRE = MR.Nodes.getNodeAs<clang::DeclRefExpr>("mcssspointerarithmetic");
		const MemberExpr* ME = MR.Nodes.getNodeAs<clang::MemberExpr>("mcssspointerarithmetic2");
		const Expr* E = MR.Nodes.getNodeAs<clang::Expr>("mcssspointerarithmetic3");

		if ((DRE != nullptr) && (E != nullptr))
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

				auto res2 = infer_array_type_info_from_stmt(*E, "pointer arithmetic", (*this).m_state1, DD);

				if (res2.update_declaration_flag) {
					update_declaration(*DD, Rewrite, m_state1);
				}
			}
		}
	}

private:
	Rewriter &Rewrite;
	CState1& m_state1;
};

/**********************************************************************************************************************/

class MCSSSMalloc2 : public MatchFinder::MatchCallback
{
public:
	MCSSSMalloc2 (Rewriter &Rewrite, CState1& state1) :
		Rewrite(Rewrite), m_state1(state1) {}

	virtual void run(const MatchFinder::MatchResult &MR)
	{
		const BinaryOperator* BO = MR.Nodes.getNodeAs<clang::BinaryOperator>("mcsssmalloc1");
		const Expr* LHS = nullptr;
		if (BO != nullptr) {
			LHS = BO->getLHS();
		}
		const CallExpr* CE = MR.Nodes.getNodeAs<clang::CallExpr>("mcsssmalloc2");
		const DeclRefExpr* DRE = MR.Nodes.getNodeAs<clang::DeclRefExpr>("mcsssmalloc3");
		const MemberExpr* ME = MR.Nodes.getNodeAs<clang::MemberExpr>("mcsssmalloc4");

		if ((BO != nullptr) && (LHS != nullptr) && (CE != nullptr) && (DRE != nullptr))
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
								std::string variable_name;
								std::string bo_replacement_code;
								const clang::DeclaratorDecl* DD = nullptr;

								auto lhs_QT = LHS->getType();

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

									auto res2 = infer_array_type_info_from_stmt(*LHS, "malloc target", (*this).m_state1, DD);

									if (res2.update_declaration_flag) {
										update_declaration(*DD, Rewrite, m_state1);
									}

									const clang::Type* lhs_TP = lhs_QT.getTypePtr();
									auto lhs_type_str = clang::QualType::getAsString(lhs_QT.split());

									std::string lhs_element_type_str;
									if (lhs_TP->isArrayType()) {
										auto ATP = llvm::cast<const clang::ArrayType>(lhs_TP);
										assert(nullptr != ATP);
										auto element_type = ATP->getElementType();
										auto elementSplitQualType = element_type.split();
										auto type_str = clang::QualType::getAsString(elementSplitQualType);
										if (("char" != type_str) && ("const char" != type_str)) {
											lhs_element_type_str = type_str;
										}
									} else if (lhs_TP->isPointerType()) {
										auto TPP = llvm::cast<const clang::PointerType>(lhs_TP);
										assert(nullptr != TPP);
										auto target_type = TPP->getPointeeType();
										auto splitQualType = target_type.split();
										auto type_str = clang::QualType::getAsString(splitQualType);
										if (("char" != type_str) && ("const char" != type_str)) {
											lhs_element_type_str = type_str;
										}
									}
									if ("" != lhs_element_type_str) {
										num_elements_text = "(";
										num_elements_text += arg_source_text;
										num_elements_text += ") / sizeof(";
										num_elements_text += lhs_element_type_str;
										num_elements_text += ")";

										auto lhs_source_range = nice_source_range(LHS->getSourceRange(), Rewrite);
										auto lhs_source_text = Rewrite.getRewrittenText(lhs_source_range);
										bo_replacement_code += "(" + lhs_source_text + ")";
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
											auto cr_shptr = std::make_shared<CMallocArray2ReplacementAction>(Rewrite, MR, CDDeclIndirection(*DD, res2.indirection_level), BO, bo_replacement_code);

											if (res2.has_been_determined_to_be_an_array) {
												(*cr_shptr).do_replacement(m_state1);
											} else {
												m_state1.m_dynamic_array2_contingent_replacement_map.insert(cr_shptr);
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

class MCSSSMallocInitializer2 : public MatchFinder::MatchCallback
{
public:
	MCSSSMallocInitializer2 (Rewriter &Rewrite, CState1& state1) :
		Rewrite(Rewrite), m_state1(state1) {}

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

								if (nullptr != DD) {
									auto decl_source_range = nice_source_range(DD->getSourceRange(), Rewrite);
									auto decl_source_location_str = decl_source_range.getBegin().printToString(*MR.SourceManager);
									std::string decl_source_text;
									if (decl_source_range.isValid()) {
										decl_source_text = Rewrite.getRewrittenText(decl_source_range);
									} else {
										return;
									}
									QualType QT = DD->getType();
									auto variable_name = DD->getNameAsString();

									auto qualified_name = DD->getQualifiedNameAsString();
									static const std::string mse_namespace_str1 = "mse::";
									static const std::string mse_namespace_str2 = "::mse::";
									if ((0 == qualified_name.compare(0, mse_namespace_str1.size(), mse_namespace_str1))
											|| (0 == qualified_name.compare(0, mse_namespace_str2.size(), mse_namespace_str2))) {
										return;
									}

									auto res1 = (*this).m_state1.m_ddecl_conversion_state_map.insert(*DD);
									auto ddcs_map_iter = res1.first;
									auto& ddcs_ref = (*ddcs_map_iter).second;
									bool update_declaration_flag = res1.second;

									bool lhs_has_been_determined_to_be_an_array = false;
									if ("native pointer" == ddcs_ref.m_indirection_state_stack[0].m_current) {
										ddcs_ref.m_indirection_state_stack[0].m_current = "malloc target";
									} else if ("inferred array" == ddcs_ref.m_indirection_state_stack[0].m_current) {
										ddcs_ref.m_indirection_state_stack[0].m_current = "dynamic array";
										lhs_has_been_determined_to_be_an_array = true;
										//update_declaration_flag = true;
										m_state1.m_dynamic_array2_contingent_replacement_map.do_and_dispose_matching_replacements(m_state1, CDDeclIndirection(*DD, 0));
									} else if ("dynamic array" == ddcs_ref.m_indirection_state_stack[0].m_current) {
										lhs_has_been_determined_to_be_an_array = true;
									} else {
										assert("native array" != ddcs_ref.m_indirection_state_stack[0].m_current);
									}

									const clang::Type* TP = QT.getTypePtr();
									auto lhs_type_str = QT.getAsString();

									std::string element_type_str;
									if (TP->isArrayType()) {
										auto ATP = llvm::cast<const clang::ArrayType>(TP);
										assert(nullptr != ATP);
										auto element_type = ATP->getElementType();
										auto elementSplitQualType = element_type.split();
										element_type_str = clang::QualType::getAsString(elementSplitQualType);
									} else if (TP->isPointerType()) {
										auto TPP = llvm::cast<const clang::PointerType>(TP);
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

										std::string initializer_info_str = "(" + num_elements_text + ")";

										auto decl_source_location_str = decl_source_range.getBegin().printToString(*MR.SourceManager);
										std::string decl_source_text;
										if (decl_source_range.isValid()) {
											decl_source_text = Rewrite.getRewrittenText(decl_source_range);
										} else {
											return;
										}

										auto DSSR = clang::SourceRange(DSSL, DSSLE);
										if (ConvertToSCPP && decl_source_range.isValid() && (DSSR.isValid())) {
											auto cr_shptr = std::make_shared<CMallocInitializerArray2ReplacementAction>(Rewrite, MR, CDDeclIndirection(*DD, 0/*indirection_level*/), DS, initializer_info_str);

											if (lhs_has_been_determined_to_be_an_array) {
												(*cr_shptr).do_replacement(m_state1);
											} else {
												m_state1.m_dynamic_array2_contingent_replacement_map.insert(cr_shptr);
											}

											int q = 3;
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

class MCSSSFree2 : public MatchFinder::MatchCallback
{
public:
	MCSSSFree2 (Rewriter &Rewrite, CState1& state1) :
		Rewrite(Rewrite), m_state1(state1) {}

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

								auto res2 = infer_array_type_info_from_stmt(*(*(CE->arg_begin())), "malloc target", (*this).m_state1, DD);

								if (res2.update_declaration_flag) {
									update_declaration(*DD, Rewrite, m_state1);
								}

								ce_replacement_code = "(" + arg_source_text + ")";
								ce_replacement_code += ".resize(0)";

								if (ConvertToSCPP && decl_source_range.isValid() && (CESR.isValid())) {
									auto cr_shptr = std::make_shared<CFreeDynamicArray2ReplacementAction>(Rewrite, MR, CDDeclIndirection(*DD, res2.indirection_level), CE, ce_replacement_code);

									if (res2.has_been_determined_to_be_an_array) {
										(*cr_shptr).do_replacement(m_state1);
									} else {
										m_state1.m_dynamic_array2_contingent_replacement_map.insert(cr_shptr);
									}
								} else {
									int q = 7;
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

class MCSSSSetToNull2 : public MatchFinder::MatchCallback
{
public:
	MCSSSSetToNull2 (Rewriter &Rewrite, CState1& state1) :
		Rewrite(Rewrite), m_state1(state1) {}

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

						auto res2 = infer_array_type_info_from_stmt(*LHS, "set to null", (*this).m_state1, DD);

						if (res2.update_declaration_flag) {
							update_declaration(*DD, Rewrite, m_state1);
						}

						auto lhs_source_range = nice_source_range(LHS->getSourceRange(), Rewrite);
						auto lhs_source_text = Rewrite.getRewrittenText(lhs_source_range);
						bo_replacement_code += "(" + lhs_source_text + ")";
						bo_replacement_code += ".resize(0)";

						if (ConvertToSCPP && decl_source_range.isValid() && (BOSR.isValid())) {
							auto cr_shptr = std::make_shared<CSetArrayPointerToNull2ReplacementAction>(Rewrite, MR, CDDeclIndirection(*DD, res2.indirection_level), BO, bo_replacement_code);

							if (res2.has_been_determined_to_be_an_array) {
								(*cr_shptr).do_replacement(m_state1);
							} else {
								m_state1.m_dynamic_array2_contingent_replacement_map.insert(cr_shptr);
							}
						} else {
							int q = 7;
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

class MCSSSMemset : public MatchFinder::MatchCallback
{
public:
	MCSSSMemset (Rewriter &Rewrite, CState1& state1) :
		Rewrite(Rewrite), m_state1(state1) {}

	virtual void run(const MatchFinder::MatchResult &MR)
	{
		const CallExpr* CE = MR.Nodes.getNodeAs<clang::CallExpr>("mcsssmemset1");
		const DeclRefExpr* DRE = MR.Nodes.getNodeAs<clang::DeclRefExpr>("mcsssmemset2");
		const MemberExpr* ME = MR.Nodes.getNodeAs<clang::MemberExpr>("mcsssmemset3");

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
			if (function_decl && (3 == num_args)) {
				{
					std::string function_name = function_decl->getNameAsString();
					static const std::string memset_str = "memset";
					if (memset_str == function_name) {
						auto iter1 = CE->arg_begin();
						assert((*iter1)->getType().getTypePtrOrNull());
						auto arg_source_range1 = nice_source_range((*iter1)->getSourceRange(), Rewrite);

						auto iter2 = iter1;
						iter2++;
						assert((*iter2)->getType().getTypePtrOrNull());
						auto arg_source_range2 = nice_source_range((*iter2)->getSourceRange(), Rewrite);

						auto iter3 = iter2;
						iter3++;
						assert((*iter3)->getType().getTypePtrOrNull());
						auto arg_source_range3 = nice_source_range((*iter3)->getSourceRange(), Rewrite);

						std::string arg_source_text1;
						std::string arg_source_text2;
						std::string arg_source_text3;
						if (arg_source_range1.isValid() && arg_source_range2.isValid() && arg_source_range3.isValid()) {
							arg_source_text1 = Rewrite.getRewrittenText(arg_source_range1);
							arg_source_text2 = Rewrite.getRewrittenText(arg_source_range2);
							arg_source_text3 = Rewrite.getRewrittenText(arg_source_range3);
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
								auto qtype_str = QT.getAsString();
								variable_name = DD->getNameAsString();

								auto qualified_name = DD->getQualifiedNameAsString();
								static const std::string mse_namespace_str1 = "mse::";
								static const std::string mse_namespace_str2 = "::mse::";
								if ((0 == qualified_name.compare(0, mse_namespace_str1.size(), mse_namespace_str1))
										|| (0 == qualified_name.compare(0, mse_namespace_str2.size(), mse_namespace_str2))) {
									return;
								}

								auto res2 = infer_array_type_info_from_stmt(*(*(CE->arg_begin())), "memset/cpy target", (*this).m_state1, DD);

								if (res2.update_declaration_flag) {
									update_declaration(*DD, Rewrite, m_state1);
								}

								const clang::Type* arg1_TP = QT.getTypePtr();
								auto arg1_type_str = QT.getAsString();

								std::string arg1_element_type_str;
								if (arg1_TP->isArrayType()) {
									auto ATP = llvm::cast<const clang::ArrayType>(arg1_TP);
									assert(nullptr != ATP);
									auto element_type = ATP->getElementType();
									auto elementSplitQualType = element_type.split();
									auto type_str = clang::QualType::getAsString(elementSplitQualType);
									if (("char" != type_str) && ("const char" != type_str)) {
										arg1_element_type_str = type_str;
									}
								} else if (arg1_TP->isPointerType()) {
									auto TPP = llvm::cast<const clang::PointerType>(arg1_TP);
									assert(nullptr != TPP);
									auto target_type = TPP->getPointeeType();
									auto splitQualType = target_type.split();
									auto type_str = clang::QualType::getAsString(splitQualType);
									if (("char" != type_str) && ("const char" != type_str)) {
										arg1_element_type_str = type_str;
									}
								}
								if ("" != arg1_element_type_str) {
									ce_replacement_code = "for (size_t i = 0; i < (" + arg_source_text3
											+ ")/sizeof(" + arg1_element_type_str + "); i += 1) { ";
									ce_replacement_code += "(" + arg_source_text1 + ")[i] = " + arg_source_text2 + "; ";
									ce_replacement_code += "}";

									if (ConvertToSCPP && decl_source_range.isValid() && (CESR.isValid())) {
										auto cr_shptr = std::make_shared<CMemsetArray2ReplacementAction>(Rewrite, MR, CDDeclIndirection(*DD, res2.indirection_level), CE, ce_replacement_code);

										if (true || res2.has_been_determined_to_be_an_array) {
											(*cr_shptr).do_replacement(m_state1);
										} else {
											m_state1.m_array2_contingent_replacement_map.insert(cr_shptr);
										}
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

private:
	Rewriter &Rewrite;
	CState1& m_state1;
};

class MCSSSMemcpy : public MatchFinder::MatchCallback
{
public:
	MCSSSMemcpy (Rewriter &Rewrite, CState1& state1) :
		Rewrite(Rewrite), m_state1(state1) {}

	virtual void run(const MatchFinder::MatchResult &MR)
	{
		const CallExpr* CE = MR.Nodes.getNodeAs<clang::CallExpr>("mcsssmemcpy1");
		const DeclRefExpr* DRE = MR.Nodes.getNodeAs<clang::DeclRefExpr>("mcsssmemcpy2");
		const MemberExpr* ME = MR.Nodes.getNodeAs<clang::MemberExpr>("mcsssmemcpy3");

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
			if (function_decl && (3 == num_args)) {
				{
					std::string function_name = function_decl->getNameAsString();
					static const std::string memcpy_str = "memcpy";
					if (memcpy_str == function_name) {
						auto iter1 = CE->arg_begin();
						assert((*iter1)->getType().getTypePtrOrNull());
						auto arg_source_range1 = nice_source_range((*iter1)->getSourceRange(), Rewrite);

						auto iter2 = iter1;
						iter2++;
						assert((*iter2)->getType().getTypePtrOrNull());
						auto arg_source_range2 = nice_source_range((*iter2)->getSourceRange(), Rewrite);

						auto iter3 = iter2;
						iter3++;
						assert((*iter3)->getType().getTypePtrOrNull());
						auto arg_source_range3 = nice_source_range((*iter3)->getSourceRange(), Rewrite);

						std::string arg_source_text1;
						std::string arg_source_text2;
						std::string arg_source_text3;
						if (arg_source_range1.isValid() && arg_source_range2.isValid() && arg_source_range3.isValid()) {
							arg_source_text1 = Rewrite.getRewrittenText(arg_source_range1);
							arg_source_text2 = Rewrite.getRewrittenText(arg_source_range2);
							arg_source_text3 = Rewrite.getRewrittenText(arg_source_range3);
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
								auto qtype_str = QT.getAsString();
								variable_name = DD->getNameAsString();

								auto qualified_name = DD->getQualifiedNameAsString();
								static const std::string mse_namespace_str1 = "mse::";
								static const std::string mse_namespace_str2 = "::mse::";
								if ((0 == qualified_name.compare(0, mse_namespace_str1.size(), mse_namespace_str1))
										|| (0 == qualified_name.compare(0, mse_namespace_str2.size(), mse_namespace_str2))) {
									return;
								}

								auto res2 = infer_array_type_info_from_stmt(*(*(CE->arg_begin())), "memset/cpy target", (*this).m_state1, DD);

								if (res2.update_declaration_flag) {
									update_declaration(*DD, Rewrite, m_state1);
								}

								const clang::Type* arg1_TP = QT.getTypePtr();
								auto arg1_type_str = QT.getAsString();

								std::string arg1_element_type_str;
								if (arg1_TP->isArrayType()) {
									auto ATP = llvm::cast<const clang::ArrayType>(arg1_TP);
									assert(nullptr != ATP);
									auto element_type = ATP->getElementType();
									auto elementSplitQualType = element_type.split();
									auto type_str = clang::QualType::getAsString(elementSplitQualType);
									if (("char" != type_str) && ("const char" != type_str)) {
										arg1_element_type_str = type_str;
									}
								} else if (arg1_TP->isPointerType()) {
									auto TPP = llvm::cast<const clang::PointerType>(arg1_TP);
									assert(nullptr != TPP);
									auto target_type = TPP->getPointeeType();
									auto splitQualType = target_type.split();
									auto type_str = clang::QualType::getAsString(splitQualType);
									if (("char" != type_str) && ("const char" != type_str)) {
										arg1_element_type_str = type_str;
									}
								}
								if ("" != arg1_element_type_str) {
									ce_replacement_code = "for (size_t i = 0; i < (" + arg_source_text3
											+ ")/sizeof(" + arg1_element_type_str + "); i += 1) { ";
									ce_replacement_code += "(" + arg_source_text1 + ")[i] = (" + arg_source_text2 + ")[i]; ";
									ce_replacement_code += "}";

									if (ConvertToSCPP && decl_source_range.isValid() && (CESR.isValid())) {
										auto cr_shptr = std::make_shared<CMemcpyArray2ReplacementAction>(Rewrite, MR, CDDeclIndirection(*DD, res2.indirection_level), CE, ce_replacement_code);

										if (true || res2.has_been_determined_to_be_an_array) {
											(*cr_shptr).do_replacement(m_state1);
										} else {
											m_state1.m_array2_contingent_replacement_map.insert(cr_shptr);
										}
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

private:
	Rewriter &Rewrite;
	CState1& m_state1;
};

/* This class addresses the initialized declarations in the form "type var = cond ? lhs : rhs;". */
class MCSSSConditionalInitializer : public MatchFinder::MatchCallback
{
public:
	MCSSSConditionalInitializer (Rewriter &Rewrite, CState1& state1) :
		Rewrite(Rewrite), m_state1(state1) {}

	virtual void run(const MatchFinder::MatchResult &MR)
	{
		const DeclStmt* DS = MR.Nodes.getNodeAs<clang::DeclStmt>("mcsssconditionalinitializer1");
		const clang::ConditionalOperator* CO = MR.Nodes.getNodeAs<clang::ConditionalOperator>("mcsssconditionalinitializer2");
		const Expr* LHS = nullptr;
		const Expr* RHS = nullptr;
		if (CO) {
			LHS = CO->getLHS();
			RHS = CO->getRHS();
		}
		const DeclaratorDecl* DD = MR.Nodes.getNodeAs<clang::DeclaratorDecl>("mcsssconditionalinitializer3");

		if ((DS != nullptr) && (LHS != nullptr) && (RHS != nullptr) && (DD != nullptr))
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

			auto decl_source_range = nice_source_range(DD->getSourceRange(), Rewrite);
			auto decl_source_location_str = decl_source_range.getBegin().printToString(*MR.SourceManager);
			std::string decl_source_text;
			if (decl_source_range.isValid()) {
				decl_source_text = Rewrite.getRewrittenText(decl_source_range);
			} else {
				return;
			}
			QualType QT = DD->getType();
			auto variable_name = DD->getNameAsString();

			auto qualified_name = DD->getQualifiedNameAsString();
			static const std::string mse_namespace_str1 = "mse::";
			static const std::string mse_namespace_str2 = "::mse::";
			if ((0 == qualified_name.compare(0, mse_namespace_str1.size(), mse_namespace_str1))
					|| (0 == qualified_name.compare(0, mse_namespace_str2.size(), mse_namespace_str2))) {
				return;
			}

			std::string var_current_state_str;
			{
				auto res1 = m_state1.m_ddecl_conversion_state_map.insert(*DD);
				auto ddcs_map_iter = res1.first;
				auto& ddcs_ref = (*ddcs_map_iter).second;
				if (1 <= ddcs_ref.m_indirection_state_stack.size()) {
					var_current_state_str = ddcs_ref.m_indirection_state_stack[0].m_current;
				} else {
					int q = 7;
				}
			}
			bool var_has_been_determined_to_be_an_array = false;
			if (("inferred array" == var_current_state_str) ||
					("dynamic array" == var_current_state_str) ||
					("native array" == var_current_state_str)) {
				if ("native array" == var_current_state_str) {
					assert(false); /* right? */
				}
				var_has_been_determined_to_be_an_array = true;
			}

			auto lhs_res2 = infer_array_type_info_from_stmt(*LHS, "", (*this).m_state1);
			auto rhs_res2 = infer_array_type_info_from_stmt(*RHS, "", (*this).m_state1);
			bool lhs_qualifies = false;
			bool rhs_qualifies = false;

			{
				auto& res2 = lhs_res2;
				if (res2.ddecl_cptr && res2.declaration_expr_cptr) {
					std::string variable_name = res2.ddecl_cptr->getNameAsString();
					auto QT = res2.ddecl_cptr->getType();
					auto LHS_QT = LHS->getType();
					/* Currently we only support the case where the value expressions are direct
					 * references to declared variables. */
					if ((QT == LHS_QT)/* && (1 == res2.indirection_level)*/) {
						lhs_qualifies = true;
						if (ConvertToSCPP) {
							/* Here we're establishing and "enforcing" the constraint that the lhs value must
							 * be of an (array) type that can be assigned to the target variable. */
							auto cr_shptr = std::make_shared<CAssignedFromArray2ReplacementAction>(Rewrite, MR, CDDeclIndirection(*DD, 0), CDDeclIndirection(*(res2.ddecl_cptr) , res2.indirection_level));

							if (var_has_been_determined_to_be_an_array) {
								(*cr_shptr).do_replacement(m_state1);
							} else {
								m_state1.m_array2_contingent_replacement_map.insert(cr_shptr);
							}
						}
					}
				}
			}

			{
				auto& res2 = rhs_res2;
				if (res2.ddecl_cptr && res2.declaration_expr_cptr) {
					std::string variable_name = res2.ddecl_cptr->getNameAsString();
					auto QT = res2.ddecl_cptr->getType();
					auto RHS_QT = RHS->getType();
					/* Currently we only support the case where the value expressions are direct
					 * references to declared variables. */
					if (QT == RHS_QT) {
						rhs_qualifies = true;
						if (ConvertToSCPP) {
							/* Here we're establishing and "enforcing" the constraint that the rhs value must
							 * be of an (array) type that can be assigned to the target variable. */
							auto cr_shptr = std::make_shared<CAssignedFromArray2ReplacementAction>(Rewrite, MR, CDDeclIndirection(*DD, 0), CDDeclIndirection(*(res2.ddecl_cptr) , res2.indirection_level));

							if (var_has_been_determined_to_be_an_array) {
								(*cr_shptr).do_replacement(m_state1);
							} else {
								m_state1.m_array2_contingent_replacement_map.insert(cr_shptr);
							}
						}
					}
				}
			}

			if (lhs_qualifies && rhs_qualifies) {
				std::string lhs_current_state_str;
				{
					auto res1 = m_state1.m_ddecl_conversion_state_map.insert(*(lhs_res2.ddecl_cptr));
					auto ddcs_map_iter = res1.first;
					auto& ddcs_ref = (*ddcs_map_iter).second;
					if (1 <= ddcs_ref.m_indirection_state_stack.size()) {
						lhs_current_state_str = ddcs_ref.m_indirection_state_stack[0].m_current;
					} else {
						int q = 7;
					}
				}
				std::string rhs_current_state_str;
				{
					auto res1 = m_state1.m_ddecl_conversion_state_map.insert(*(rhs_res2.ddecl_cptr));
					auto ddcs_map_iter = res1.first;
					auto& ddcs_ref = (*ddcs_map_iter).second;
					if (1 <= ddcs_ref.m_indirection_state_stack.size()) {
						rhs_current_state_str = ddcs_ref.m_indirection_state_stack[0].m_current;
					} else {
						int q = 7;
					}
				}

				if (ConvertToSCPP) {
					/* Here we're establishing and "enforcing" the constraint that the lhs and rhs
					 * values of the conditional operator must be the same type. */
					{
						auto cr_shptr = std::make_shared<CConditionalOperatorReconciliation2ReplacementAction>(Rewrite, MR, CDDeclIndirection(*lhs_res2.ddecl_cptr, 0), CO, lhs_res2.ddecl_cptr, rhs_res2.ddecl_cptr, DD);

						if ("dynamic array" == lhs_current_state_str) {
							(*cr_shptr).do_replacement(m_state1);
						} else if ("native array" == lhs_current_state_str) {
							(*cr_shptr).do_replacement(m_state1);
						} else {
							m_state1.m_dynamic_array2_contingent_replacement_map.insert(cr_shptr);
							if ("inferred array" == lhs_current_state_str) {
								(*cr_shptr).do_replacement(m_state1);
							} else {
								m_state1.m_array2_contingent_replacement_map.insert(cr_shptr);
							}
						}
					}
					{
						auto cr_shptr = std::make_shared<CConditionalOperatorReconciliation2ReplacementAction>(Rewrite, MR, CDDeclIndirection(*rhs_res2.ddecl_cptr, 0), CO, lhs_res2.ddecl_cptr, rhs_res2.ddecl_cptr, DD);

						if ("dynamic array" == rhs_current_state_str) {
							(*cr_shptr).do_replacement(m_state1);
						} else if ("native array" == rhs_current_state_str) {
							(*cr_shptr).do_replacement(m_state1);
						} else {
							m_state1.m_dynamic_array2_contingent_replacement_map.insert(cr_shptr);
							if ("inferred array" == rhs_current_state_str) {
								(*cr_shptr).do_replacement(m_state1);
							} else {
								m_state1.m_array2_contingent_replacement_map.insert(cr_shptr);
							}
						}
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
class MyASTConsumer : public ASTConsumer {

public:
  MyASTConsumer(Rewriter &R) : HandlerForSSSNativePointer(R), HandlerForSSSArrayToPointerDecay(R, m_state1),
	HandlerForSSSVarDecl2(R, m_state1), HandlerForSSSPointerArithmetic2(R, m_state1), HandlerForSSSMalloc2(R, m_state1),
	HandlerForSSSMallocInitializer2(R, m_state1), HandlerForSSSFree2(R, m_state1), HandlerForSSSSetToNull2(R, m_state1),
	HandlerForSSSMemset(R, m_state1), HandlerForSSSMemcpy(R, m_state1), HandlerForSSSConditionalInitializer(R, m_state1)
  {
	  Matcher.addMatcher(varDecl(hasType(pointerType())).bind("mcsssnativepointer"), &HandlerForSSSNativePointer);

	  Matcher.addMatcher(castExpr(allOf(hasCastKind(CK_ArrayToPointerDecay), unless(hasParent(arraySubscriptExpr())))).bind("mcsssarraytopointerdecay"), &HandlerForSSSArrayToPointerDecay);

	  Matcher.addMatcher(clang::ast_matchers::declaratorDecl().bind("mcsssvardecl"), &HandlerForSSSVarDecl2);

	  Matcher.addMatcher(expr(allOf(
	  		hasParent(expr(anyOf( \
					unaryOperator(hasOperatorName("++")), unaryOperator(hasOperatorName("--")), \
					binaryOperator(hasOperatorName("+=")), binaryOperator(hasOperatorName("-=")),
					castExpr(hasParent(expr(anyOf(
							binaryOperator(hasOperatorName("+")), binaryOperator(hasOperatorName("+=")),
							binaryOperator(hasOperatorName("-")), binaryOperator(hasOperatorName("-=")),
							binaryOperator(hasOperatorName("<=")), binaryOperator(hasOperatorName("<")),
							binaryOperator(hasOperatorName(">=")), binaryOperator(hasOperatorName(">")),
							arraySubscriptExpr()/*, clang::ast_matchers::castExpr(hasParent(arraySubscriptExpr()))*/
					))))))),
				hasType(pointerType()),
				anyOf(
						memberExpr(expr(hasDescendant(declRefExpr().bind("mcssspointerarithmetic")))).bind("mcssspointerarithmetic2"),
						declRefExpr().bind("mcssspointerarithmetic"),
						hasDescendant(memberExpr(expr(hasDescendant(declRefExpr().bind("mcssspointerarithmetic")))).bind("mcssspointerarithmetic2")),
						hasDescendant(declRefExpr().bind("mcssspointerarithmetic"))
				)
				)).bind("mcssspointerarithmetic3"), &HandlerForSSSPointerArithmetic2);

	  Matcher.addMatcher(binaryOperator(allOf(
	  		hasOperatorName("="),
	  		hasRHS(
	  				anyOf(
	  						cStyleCastExpr(has(callExpr().bind("mcsssmalloc2"))),
								callExpr().bind("mcsssmalloc2")
	  				)
				),
	  		hasLHS(anyOf(
						memberExpr(expr(hasDescendant(declRefExpr().bind("mcsssmalloc3")))).bind("mcsssmalloc4"),
						declRefExpr().bind("mcsssmalloc3"),
						hasDescendant(memberExpr(expr(hasDescendant(declRefExpr().bind("mcsssmalloc3")))).bind("mcsssmalloc4")),
						hasDescendant(declRefExpr().bind("mcsssmalloc3"))
				)),
	  		hasLHS(expr(hasType(pointerType())))
				)).bind("mcsssmalloc1"), &HandlerForSSSMalloc2);

	  Matcher.addMatcher(declStmt(hasDescendant(
	  		varDecl(hasInitializer(ignoringImpCasts(
					anyOf(
							cStyleCastExpr(has(callExpr().bind("mcsssmallocinitializer2"))),
							callExpr().bind("mcsssmallocinitializer2")
					)
	  		))).bind("mcsssmallocinitializer3")
				)).bind("mcsssmallocinitializer1"), &HandlerForSSSMallocInitializer2);

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
	  		)).bind("mcsssfree1"), &HandlerForSSSFree2);

	  Matcher.addMatcher(binaryOperator(allOf(
	  		hasOperatorName("="),
	  		hasLHS(anyOf(
	  				memberExpr(expr(hasDescendant(declRefExpr().bind("mcssssettonull3")))).bind("mcssssettonull4"),
	  				hasDescendant(memberExpr(expr(hasDescendant(declRefExpr().bind("mcssssettonull3")))).bind("mcssssettonull4")),
						hasDescendant(declRefExpr().bind("mcssssettonull3"))
				)),
				hasLHS(expr(hasType(pointerType())))
				)).bind("mcssssettonull1"), &HandlerForSSSSetToNull2);

	  Matcher.addMatcher(
	  		callExpr(allOf(
	  				hasAnyArgument(
	  				expr(anyOf(
								memberExpr(expr(hasDescendant(declRefExpr().bind("mcsssmemset2")))).bind("mcsssmemset3"),
								hasDescendant(memberExpr(expr(hasDescendant(declRefExpr().bind("mcsssmemset2")))).bind("mcsssmemset3")),
								hasDescendant(declRefExpr().bind("mcsssmemset2"))
						))),
						argumentCountIs(3),
						hasAnyArgument(hasType(pointerType()))
	  		)).bind("mcsssmemset1"), &HandlerForSSSMemset);

	  Matcher.addMatcher(
	  		callExpr(allOf(
	  				hasAnyArgument(
	  				expr(anyOf(
								memberExpr(expr(hasDescendant(declRefExpr().bind("mcsssmemcpy2")))).bind("mcsssmemcpy3"),
								hasDescendant(memberExpr(expr(hasDescendant(declRefExpr().bind("mcsssmemcpy2")))).bind("mcsssmemcpy3")),
								hasDescendant(declRefExpr().bind("mcsssmemcpy2"))
						))),
						argumentCountIs(3),
						hasAnyArgument(hasType(pointerType()))
	  		)).bind("mcsssmemcpy1"), &HandlerForSSSMemcpy);

	  Matcher.addMatcher(declStmt(hasDescendant(
	  		varDecl(hasInitializer(ignoringImpCasts(
					anyOf(
							conditionalOperator(has(declRefExpr())).bind("mcsssconditionalinitializer2"),
							conditionalOperator(hasDescendant(declRefExpr())).bind("mcsssconditionalinitializer2"),
							cStyleCastExpr(has(conditionalOperator(has(declRefExpr())).bind("mcsssconditionalinitializer2"))),
							cStyleCastExpr(has(conditionalOperator(hasDescendant(declRefExpr())).bind("mcsssconditionalinitializer2")))
					)
	  		))).bind("mcsssconditionalinitializer3")
				)).bind("mcsssconditionalinitializer1"), &HandlerForSSSConditionalInitializer);

  }

  void HandleTranslationUnit(ASTContext &Context) override 
  {
    Matcher.matchAST(Context);
  }

private:

  CState1 m_state1;

  MCSSSNativePointer HandlerForSSSNativePointer;
  MCSSSArrayToPointerDecay HandlerForSSSArrayToPointerDecay;
  MCSSSVarDecl2 HandlerForSSSVarDecl2;
  MCSSSPointerArithmetic2 HandlerForSSSPointerArithmetic2;
  MCSSSMalloc2 HandlerForSSSMalloc2;
  MCSSSMallocInitializer2 HandlerForSSSMallocInitializer2;
  MCSSSFree2 HandlerForSSSFree2;
  MCSSSSetToNull2 HandlerForSSSSetToNull2;
  MCSSSMemset HandlerForSSSMemset;
  MCSSSMemcpy HandlerForSSSMemcpy;
  MCSSSConditionalInitializer HandlerForSSSConditionalInitializer;

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

