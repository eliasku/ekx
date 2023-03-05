import {AppLib} from "./app";
import {LibTextureLoader} from "./texture-loader";
import {LocalResLib} from "./res";

mergeInto(LibraryManager.library, AppLib);
mergeInto(LibraryManager.library, LocalResLib);
mergeInto(LibraryManager.library, LibTextureLoader);
