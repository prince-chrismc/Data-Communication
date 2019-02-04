/*

MIT License

Copyright (c) 2018 Chris McArthur, prince.chrismc(at)gmail(dot)com

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/

#pragma once

namespace Http
{
   enum class Version
   {
      Invalid = 0,
      v10,
      v11,
   };

   enum class Status
   {
      Invalid,
      Ok = 200,
      Created = 201,
      Accepted = 202,
      NoContent = 204,
      PartialContent = 206,
      BadRequest = 400,
      Forbidden = 403,
      NotFound = 404,
      MethodNotAllowed = 405,
      Conflict = 409,
      PreConditionFailed = 412,
      RequestEntityTooLarge = 413,
      UnsupportedMediaType = 415,
      RequestedRangeNotSatisfiable = 416,
      InternalServerError = 500,
      NotImplemented = 501,
      HttpVersionNotSupported = 505
   };

   enum class ContentType
   {
      Invalid,
      Text,
      Html,
      Json,
      Yaml,
      Xml,
      Gif,
      Ico,
      Png
   };

   enum class RequestMethod
   {
      Invalid = 0,
      Options,
      Get,
      Head,
      Post,
      Put,
      Delete,
      Trace,
      Connect,
      Patch,
      Last
   };

}
