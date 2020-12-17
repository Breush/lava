#pragma once

namespace lava::magma::vulkan {
    template<class UniqueT> inline
    UniqueT&& checkMove(vk::ResultValue<UniqueT>& resultValue,
                        const char* errorTitle, const char* errorMessage) {
        using namespace std::string_literals;

        if (resultValue.result != vk::Result::eSuccess) {
            lava::chamber::logger.warning("magma.vulkan."s + errorTitle) << "Result is: " << vk::to_string(resultValue.result) << std::endl;
            lava::chamber::logger.error("magma.vulkan."s + errorTitle) << errorMessage << std::endl;
        }

        return std::move(resultValue.value);
    }
}
