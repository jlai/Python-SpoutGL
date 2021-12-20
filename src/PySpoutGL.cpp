/*
 * Copyright 2021 Google LLC
 * 
 *  Use of this source code is governed by a BSD-style
 *  license that can be found in the LICENSE file or at
 *  https://developers.google.com/open-source/licenses/bsd
 */

#include <codeanalysis/warnings.h>

#pragma warning(push)
#pragma warning(disable: ALL_CODE_ANALYSIS_WARNINGS)

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "SpoutSender.h"
#include "SpoutReceiver.h"
#include <gl/gl.h>
#include <cstdint>

#pragma warning(pop)

#define STRINGIFY(x) #x
#define MACRO_STRINGIFY(x) STRINGIFY(x)

static_assert(sizeof(float) == 4, "expected 4 byte float");

namespace py = pybind11;

template<typename ByteType>
void copyToFloatBuffer(py::buffer_info& src, py::buffer_info& dest) {
    ByteType* values = static_cast<ByteType *>(src.ptr);
    float* floats = static_cast<float *>(dest.ptr);
    
    int bitsPerEntry = 8 * sizeof(ByteType);
    float maxValue = (float) ((1 << bitsPerEntry) - 1);

    for (int i = 0; i < src.size; i++) {
        floats[i] = float(values[i]) / maxValue;
    }
}

template<typename ByteType>
void copyToByteBuffer(py::buffer_info& src, py::buffer_info& dest) {
    float* floats = static_cast<float *>(src.ptr);
    ByteType* bytes = static_cast<ByteType *>(dest.ptr);
    
    int bitsPerEntry = 8 * sizeof(ByteType);
    ByteType maxValue = (ByteType) ((1 << bitsPerEntry) - 1);

    for (int i = 0; i < dest.size; i++) {
        bytes[i] = (ByteType) (floats[i] * maxValue);
    }
}

int getBytesPerPixel(GLenum format) {
    switch (format) {
    case GL_RGBA:
    case GL_BGRA_EXT:
        return 4;
    case GL_RGB:
    case GL_BGR_EXT:
        return 3;
    default:
        throw std::invalid_argument("Unknown GL format");
    }
}

template<typename T>
py::class_<T> &addCommonDefs(py::class_<T> &clazz) {
    return clazz
        .def("isGLDXReady", &T::IsGLDXready)
        .def("getAutoShare", &T::GetAutoShare)
        .def("setAutoShare", &T::SetAutoShare)
        .def("getCPUshare", &T::GetCPUshare)
        .def("setCPUshare", &T::SetCPUshare)
        .def("createOpenGL", &T::CreateOpenGL)
        .def("closeOpenGL", &T::CloseOpenGL);
}

PYBIND11_MODULE(_spoutgl, m) {
    m.doc() = R"pbdoc(
        SpoutGL for Python
        -----------------------

        .. currentmodule:: SpoutGL
    )pbdoc";
    
    addCommonDefs(py::class_<SpoutSender>(m, "SpoutSender")
        .def(py::init<>())
        .def("__enter__", [&](SpoutSender& sender) {
                return &sender;
            })
        .def("__exit__", [&](SpoutSender& sender, const py::object&, const py::object&, const py::object&) {
                sender.ReleaseSender();
            })
        .def("setSenderName", &SpoutSender::SetSenderName)
        .def("releaseSender", &SpoutSender::ReleaseSender)
        .def("sendTexture", &SpoutSender::SendTexture)
        .def("sendFbo", &SpoutSender::SendFbo)
        .def("sendImage", [](SpoutSender& sender, const py::buffer pixelBuffer, int height, int width, GLenum glFormat, bool invert, GLuint hostFbo) {
            py::buffer_info bufferInfo = pixelBuffer.request();
            if (bufferInfo.size * bufferInfo.itemsize < width * height * getBytesPerPixel(glFormat)) {
                throw pybind11::buffer_error("Buffer not large enough");
            }

            if (!PyBuffer_IsContiguous(bufferInfo.view(), 'C')) {
                throw pybind11::buffer_error("Buffer must be contiguous");
            }

            py::gil_scoped_release release; // Safe? https://mail.python.org/pipermail/python-dev/2018-July/154652.html
            return sender.SendImage((unsigned char*)bufferInfo.ptr, height, width, glFormat, invert, hostFbo);
            })

        .def("getFrame", &SpoutSender::GetFrame)
        .def("setFrameSync", &SpoutSender::SetFrameSync)
        .def("waitFrameSync", &SpoutSender::WaitFrameSync, py::call_guard<py::gil_scoped_release>())
        .def("getCPU", &SpoutSender::GetCPU)
        .def("getGLDX", &SpoutSender::GetGLDX)
   );

    addCommonDefs(py::class_<SpoutReceiver>(m, "SpoutReceiver")
        .def(py::init<>())
        .def("__enter__", [&](SpoutReceiver& receiver) {
                return &receiver;
            })
        .def("__exit__", [&](SpoutReceiver& receiver, const py::object &, const py::object &, const py::object &) {
                receiver.ReleaseReceiver();
            })
        .def("setReceiverName", &SpoutReceiver::SetReceiverName)
        .def("releaseReceiver", &SpoutReceiver::ReleaseReceiver)
        .def("receiveTexture", static_cast<bool (SpoutReceiver::*)(void)>(&SpoutReceiver::ReceiveTexture))
        .def("receiveTexture", static_cast<bool (SpoutReceiver::*)(GLuint, GLuint, bool, GLuint)>(&SpoutReceiver::ReceiveTexture))
        .def("receiveImage", [](SpoutReceiver& receiver, std::optional<py::buffer> buffer, GLenum glFormat, bool invert, GLuint hostFbo) {
            if (!buffer.has_value()) {
                // If None is passed in, wait for sender update
                return receiver.ReceiveImage(NULL, glFormat, invert, hostFbo);
            }

            py::buffer_info bufferInfo(buffer->request());

            if (bufferInfo.size == 0) {
                // If empty buffer is passed in, wait for sender update
                return receiver.ReceiveImage(NULL, glFormat, invert, hostFbo);
            }

            unsigned int width = receiver.GetSenderWidth();
            unsigned int height = receiver.GetSenderHeight();

            if (bufferInfo.size * bufferInfo.itemsize < width * height * getBytesPerPixel(glFormat)) {
                throw pybind11::buffer_error("Buffer not large enough");
            }

            if (!PyBuffer_IsContiguous(bufferInfo.view(), 'C')) {
                throw pybind11::buffer_error("Buffer must be contiguous");
            }

            py::gil_scoped_release release; // Safe? https://mail.python.org/pipermail/python-dev/2018-July/154652.html
            return receiver.ReceiveImage(static_cast<unsigned char*>(bufferInfo.ptr), glFormat, invert, hostFbo);
        })
        .def("getSenderName", &SpoutReceiver::GetSenderWidth)
        .def("getSenderWidth", &SpoutReceiver::GetSenderWidth)
        .def("getSenderHeight", &SpoutReceiver::GetSenderHeight)
        .def("getSenderFormat", &SpoutReceiver::GetSenderFormat)
        .def("isConnected", &SpoutReceiver::IsConnected)
        .def("isUpdated", &SpoutReceiver::IsUpdated)
        .def("isFrameNew", &SpoutReceiver::IsFrameNew)
        .def("isFrameCountEnabled", &SpoutReceiver::IsFrameCountEnabled)
        .def("getSenderFrame", &SpoutReceiver::GetSenderFrame)
        .def("setFrameSync", &SpoutReceiver::SetFrameSync)
        .def("waitFrameSync", &SpoutReceiver::WaitFrameSync, py::arg("senderName"), py::arg("timeout") = 0, py::call_guard<py::gil_scoped_release>())
    );

    auto enums = m.def_submodule("enums", "Re-export of supported OpenGL format enums as integers");

    enums.add_object("GL_RGBA", py::int_(GL_RGBA));
    enums.add_object("GL_BGRA_EXT", py::int_(GL_BGRA_EXT));
    enums.add_object("GL_RGB", py::int_(GL_RGB));
    enums.add_object("GL_BGR_EXT", py::int_(GL_BGR_EXT));
    
    auto helpers = m.def_submodule("helpers", "Helper utilities specific to the Python wrapper");

    helpers.def("getBytesPerPixel", &getBytesPerPixel, "Get the number of bytes per pixel for a GL format (assuming GL_UNSIGNED_BYTE)");

    helpers.def("isBufferEmpty", [](py::buffer byteBuffer) {
        py::buffer_info byteBufferInfo(byteBuffer.request());

        if (!PyBuffer_IsContiguous(byteBufferInfo.view(), 'C')) {
            throw pybind11::buffer_error("Byte buffer must be contiguous");
        }

        py::ssize_t numBytes = byteBufferInfo.size * byteBufferInfo.itemsize;
        uint8_t *bytes = static_cast<uint8_t*>(byteBufferInfo.ptr);

        for (int i = 0; i < numBytes; i++) {
            if (bytes[i] != 0) {
                return false;
            }
        }

        return true;
    }, "Check if a buffer is all zero bytes. Workaround for empty initial buffers.");

    helpers.def("copyToFloat32", [](py::buffer byteBuffer, py::buffer floatBuffer) {
        assert(sizeof(float) == 4);

        py::buffer_info byteBufferInfo(byteBuffer.request());
        py::buffer_info floatBufferInfo(floatBuffer.request());

        if (floatBufferInfo.format != "f" || floatBufferInfo.itemsize != 4) {
            throw pybind11::buffer_error("Expected float buffer");
        }

        if (floatBufferInfo.size != byteBufferInfo.size) {
            throw pybind11::buffer_error("Expected float buffer to be equal size to byte buffer");
        }

        if (!PyBuffer_IsContiguous(byteBufferInfo.view(), 'C')) {
            throw pybind11::buffer_error("Byte buffer must be contiguous");
        }

        if (!PyBuffer_IsContiguous(floatBufferInfo.view(), 'C')) {
            throw pybind11::buffer_error("Float buffer must be contiguous");
        }

        if (byteBufferInfo.itemsize == 1) {
            copyToFloatBuffer<uint8_t>(byteBufferInfo, floatBufferInfo);
        } else if (byteBufferInfo.itemsize == 2) {
            copyToFloatBuffer<uint16_t>(byteBufferInfo, floatBufferInfo);
        } else {
            throw pybind11::buffer_error("Byte buffer item size must be 1-2 bytes");
        }
    }, "Copy buffer with 8/16-bit unsigned RGB(A) byte values into a 32-bit float buffer");

    helpers.def("copyToByteBuffer", [](py::buffer floatBuffer, py::buffer byteBuffer) {
        assert(sizeof(float) == 4);

        py::buffer_info floatBufferInfo(floatBuffer.request());
        py::buffer_info byteBufferInfo(byteBuffer.request());

        if (floatBufferInfo.format != "f" || floatBufferInfo.itemsize != 4) {
            throw pybind11::buffer_error("Expected float buffer");
        }

        if (floatBufferInfo.size != byteBufferInfo.size) {
            throw pybind11::buffer_error("Expected float buffer to be equal size to byte buffer");
        }

        if (!PyBuffer_IsContiguous(byteBufferInfo.view(), 'C')) {
            throw pybind11::buffer_error("Byte buffer must be contiguous");
        }

        if (!PyBuffer_IsContiguous(floatBufferInfo.view(), 'C')) {
            throw pybind11::buffer_error("Float buffer must be contiguous");
        }

        if (byteBufferInfo.itemsize == 1) {
            copyToByteBuffer<uint8_t>(floatBufferInfo, byteBufferInfo);
        } else if (byteBufferInfo.itemsize == 2) {
            copyToFloatBuffer<uint16_t>(floatBufferInfo, byteBufferInfo);
        } else {
            throw pybind11::buffer_error("Byte buffer item size must be 1-2 bytes");
        }
    }, "Copy buffer with 32-bit RGB(A) float values into a 8/16-bit unsigned byte buffer");

#ifdef VERSION_INFO
    m.attr("__version__") = MACRO_STRINGIFY(VERSION_INFO);
#else
    m.attr("__version__") = "dev";
#endif
}
