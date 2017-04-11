
def mutator_ignore : InheritableAttr
{
  let Spellings = [CXX11<"clang", "mutator-ignore", "">];
  let Subjects = Subjectlist<[Var, ObjCIvar, Type, Enum, EnumConstant, Label, Field, ObjCMethod, FunctionLike, CXXRecord]>;
  let Documentation = [Undocumented];
  let ASTNode = 0;
  let Args = [BoolArgument<"ExtendToDeclRefExpr">];
  let Semahandler = 0;
}
