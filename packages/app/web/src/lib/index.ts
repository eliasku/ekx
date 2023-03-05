import {AppLib} from "./app";
import {LocalStorageLib} from "./local-storage";
import {LocalResLib} from "./res";
import {LibTextureLoader} from "./texture-loader";

mergeInto(LibraryManager.library, AppLib);
mergeInto(LibraryManager.library, LocalStorageLib);
mergeInto(LibraryManager.library, LocalResLib);
mergeInto(LibraryManager.library, LibTextureLoader);
