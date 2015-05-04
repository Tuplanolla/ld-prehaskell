{-# LANGUAGE ForeignFunctionInterface #-}

module Hook where

import Foreign.C.String
import Foreign.C.Types

hook :: CString -> IO CInt
hook cs = do s <- peekCString cs
             putStrLn $ "Hooked '" ++ s ++ "'."
             return 0

foreign export ccall hook :: CString -> IO CInt
