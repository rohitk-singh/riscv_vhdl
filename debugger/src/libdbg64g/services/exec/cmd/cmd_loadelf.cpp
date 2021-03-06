/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Elf-file loader command.
 */

#include "iservice.h"
#include "cmd_loadelf.h"
#include "coreservices/ielfloader.h"

namespace debugger {

CmdLoadElf::CmdLoadElf(ITap *tap, ISocInfo *info) 
    : ICommand ("loadelf", tap, info) {

    briefDescr_.make_string("Load ELF-file");
    detailedDescr_.make_string(
        "Description:\n"
        "    Load ELF-file to SOC target memory.\n"
        "Example:\n"
        "    loadelf /home/riscv/image.elf\n");
}

bool CmdLoadElf::isValid(AttributeType *args) {
    if ((*args)[0u].is_equal("loadelf") && args->size() == 2) {
        return CMD_VALID;
    }
    return CMD_INVALID;
}

void CmdLoadElf::exec(AttributeType *args, AttributeType *res) {
    res->make_nil();
    if (!isValid(args)) {
        generateError(res, "Wrong argument list");
        return;
    }

    /**
     *  @todo Elf Loader service change on elf-reader
     */
    AttributeType lstServ;
    RISCV_get_services_with_iface(IFACE_ELFLOADER, &lstServ);
    if (lstServ.size() == 0) {
        generateError(res, "Elf-service not found");
        return;
    }

    DsuMapType *dsu = info_->getpDsu();
    uint64_t soft_reset = 1;
    uint64_t addr = reinterpret_cast<uint64_t>(&dsu->ulocal.v.soft_reset);

    tap_->write(addr, 8, reinterpret_cast<uint8_t *>(&soft_reset));

    IService *iserv = static_cast<IService *>(lstServ[0u].to_iface());
    IElfLoader *elf = static_cast<IElfLoader *>(
                        iserv->getInterface(IFACE_ELFLOADER));
    elf->loadFile((*args)[1].to_string());

    soft_reset = 0;
    tap_->write(addr, 8, reinterpret_cast<uint8_t *>(&soft_reset));
}


}  // namespace debugger
