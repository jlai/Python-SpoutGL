#include <codeanalysis/warnings.h>

#pragma warning(push)
#pragma warning(disable: ALL_CODE_ANALYSIS_WARNINGS)

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "SpoutSender.h"
#include "SpoutReceiver.h"
#include <gl/gl.h>

#pragma warning(pop)

#define STRINGIFY(x) #x
#define MACRO_STRINGIFY(x) STRINGIFY(x)

namespace py = pybind11;

PYBIND11_MODULE(SpoutGL, m) {
    m.doc() = R"pbdoc(
        SpoutGL for Python
        -----------------------

        .. currentmodule:: spout

        .. autosummary::
           :toctree: _generate

           add
           subtract
    )pbdoc";
    
    py::class_<SpoutSender>(m, "SpoutSender")
        .def(py::init<>())
        .def("__enter__", [&](SpoutSender& sender) { return sender; }
            , "Enter the runtime context related to this object")
        .def("__exit__", [&](SpoutSender& sender, std::optional<py::object> exc_type, std::optional<py::object> exc_value, std::optional<py::object> traceback) { sender.ReleaseSender(); return py::none(); }
            , "Exit the runtime context related to this object")
        .def("setSenderName", &SpoutSender::SetSenderName)
        .def("releaseSender", &SpoutSender::ReleaseSender)
        .def("sendImage", [](SpoutSender &sender, const py::buffer pixelBuffer, int height, int width, GLenum glFormat, bool invert, GLuint hostFbo) {
            py::buffer_info bufferInfo = pixelBuffer.request();
            if (bufferInfo.size * bufferInfo.itemsize < width * height * 4) {
                throw pybind11::buffer_error("Buffer not large enough");
            }

            if (!PyBuffer_IsContiguous(bufferInfo.view(), 'C')) {
                throw pybind11::buffer_error("Buffer must be contiguous");
            }

            return sender.SendImage((unsigned char*) bufferInfo.ptr, height, width, glFormat, invert, hostFbo);
        })
        .def("setFrameSync", &SpoutSender::SetFrameSync)
        .def("waitFrameSync", &SpoutSender::WaitFrameSync);
        
    py::class_<SpoutReceiver>(m, "SpoutReceiver")
        .def(py::init<>())
        .def("__enter__", [&](SpoutReceiver& receiver) { return receiver; }
            , "Enter the runtime context related to this object")
        .def("__exit__", [&](SpoutReceiver& receiver, std::optional<py::object> exc_type, std::optional<py::object> exc_value, std::optional<py::object> traceback) { receiver.ReleaseReceiver(); return py::none(); }
            , "Exit the runtime context related to this object")
        .def("setReceiverName", &SpoutReceiver::SetReceiverName)
        .def("releaseReceiver", &SpoutReceiver::ReleaseReceiver)
        .def("receiveTexture", static_cast<bool (SpoutReceiver::*)(void)>(&SpoutReceiver::ReceiveTexture))
        .def("receiveTexture", static_cast<bool (SpoutReceiver::*)(GLuint, GLuint, bool, GLuint)>(&SpoutReceiver::ReceiveTexture))
        .def("receiveImage", [](SpoutReceiver &receiver, py::buffer buffer, GLenum glFormat, bool invert, GLuint hostFbo) {
            unsigned int width = receiver.GetSenderWidth();
            unsigned int height = receiver.GetSenderWidth();
            
            py::buffer_info bufferInfo = buffer.request(true);
            if (bufferInfo.size * bufferInfo.itemsize < width * height * 4) {
                throw pybind11::buffer_error("Buffer not large enough");
            }

            if (!PyBuffer_IsContiguous(bufferInfo.view(), 'C')) {
                throw pybind11::buffer_error("Buffer must be contiguous");
            }

            return receiver.ReceiveImage(static_cast<unsigned char*>(bufferInfo.ptr), glFormat, invert, hostFbo);
        })
        .def("getSenderWidth", &SpoutReceiver::GetSenderWidth)
        .def("getSenderHeight", &SpoutReceiver::GetSenderHeight)
        .def("getSenderFormat", &SpoutReceiver::GetSenderFormat)
        .def("isConnected", &SpoutReceiver::IsConnected)
        .def("isUpdated", &SpoutReceiver::IsUpdated)
        .def("isFrameNew", &SpoutReceiver::IsFrameNew)
        .def("isFrameCountEnabled", &SpoutReceiver::IsFrameCountEnabled)
        .def("getSenderFrame", &SpoutReceiver::GetSenderFrame)
        .def("setFrameSync", &SpoutReceiver::SetFrameSync)
        .def("waitFrameSync", &SpoutReceiver::WaitFrameSync);

#ifdef VERSION_INFO
    m.attr("__version__") = MACRO_STRINGIFY(VERSION_INFO);
#else
    m.attr("__version__") = "dev";
#endif
}

