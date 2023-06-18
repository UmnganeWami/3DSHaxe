package;

import cxx.Compiler as CxxConfig;
import cxxcompiler.Compiler as HaxeCompiler;

class Init {
	public static function init() {
		CxxConfig.setExceptionHandlingEnabled(false);
		CxxConfig.setDynamicTypeEnabled(false);
		CxxConfig.setRetainConstCharLiterals(true);

		//HaxeCompiler.OptionalClassCpp = "int";
		//HaxeCompiler.OptionalInclude = (["bn_optional.h", true] : Dynamic);
		HaxeCompiler.OptionalNullCpp = "NULL"; //so it uses NULL instead of doing std:nullopt
	}
}