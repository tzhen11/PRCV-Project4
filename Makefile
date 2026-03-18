# Detect OS
ifeq ($(OS),Windows_NT)
    # Windows settings
    CXX = g++
    OPENCV_DIR = C:/msys64/ucrt64
    ONNX_DIR = C:/onnxruntime
    CXXFLAGS = -std=c++17 -I$(OPENCV_DIR)/include/opencv4 -I$(ONNX_DIR)/include
    LDFLAGS = -L$(OPENCV_DIR)/lib -L$(ONNX_DIR)/lib
    LDFLAGS += -lopencv_core -lopencv_imgproc -lopencv_highgui -lopencv_imgcodecs -lopencv_videoio -lopencv_objdetect -lopencv_dnn -lopencv_calib3d -lopencv_features2d
    LDFLAGS += -lonnxruntime
    RM = del /Q
    EXE = .exe
else
    # macOS settings
    CXX = clang++
    CXXFLAGS = -std=c++17 $(shell pkg-config --cflags opencv4)
    CXXFLAGS += -I$(HOME)/onnxruntime/include
    LDFLAGS = $(shell pkg-config --libs opencv4)
    LDFLAGS += -L$(HOME)/onnxruntime/lib -lonnxruntime
    RM = rm -f
    EXE =
endif

# Source files
COMMON_SRC = settings.cpp

# Targets
detect: main.cpp $(COMMON_SRC)
	$(CXX) $(CXXFLAGS) main.cpp $(COMMON_SRC) -o main$(EXE) $(LDFLAGS)

ar: ar.cpp
	$(CXX) $(CXXFLAGS) ar.cpp $(COMMON_SRC) -o ar$(EXE) $(LDFLAGS)

features: features.cpp
	$(CXX) $(CXXFLAGS) features.cpp ${COMMON_SRC} -o features$(EXE) $(LDFLAGS)

clean:
	$(RM) main$(EXE)

.PHONY: clean