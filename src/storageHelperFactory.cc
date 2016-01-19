/**
 * @file storageHelperFactory.cc
 * @author Rafal Slota
 * @copyright (C) 2013 ACK CYFRONET AGH
 * @copyright This software is released under the MIT license cited in
 * 'LICENSE.txt'
 */

#include "helpers/storageHelperFactory.h"

#include "cephHelper.h"
#include "directIOHelper.h"
#include "s3Helper.h"

#ifdef BUILD_PROXY_IO
#include "proxyIOHelper.h"
#endif

namespace one {
namespace helpers {

#ifdef BUILD_PROXY_IO
StorageHelperFactory::StorageHelperFactory(asio::io_service &cephService,
    asio::io_service &dioService, asio::io_service &s3Service,
    communication::Communicator &communicator)
    : m_cephService{cephService}
    , m_dioService{dioService}
    , m_s3Service{s3Service}
    , m_communicator{communicator}
{
}
#else
StorageHelperFactory::StorageHelperFactory(asio::io_service &ceph_service,
    asio::io_service &dio_service, asio::io_service &s3Service)
    : m_cephService{ceph_service}
    , m_dioService{dio_service}
    , m_s3Service{s3Service}
{
}
#endif

std::shared_ptr<IStorageHelper> StorageHelperFactory::getStorageHelper(
    const std::string &sh_name,
    const std::unordered_map<std::string, std::string> &args)
{
    if (sh_name == "Ceph")
        return std::make_shared<CephHelper>(args, m_cephService);

#ifdef BUILD_PROXY_IO
    if (sh_name == "ProxyIO")
        return std::make_shared<ProxyIOHelper>(args, m_communicator);
#endif

    if (sh_name == "DirectIO") {
#ifdef __linux__
        auto userCTXFactory = DirectIOHelper::linuxUserCTXFactory;
#else
        auto userCTXFactory = DirectIOHelper::noopUserCTXFactory;
#endif
        return std::make_shared<DirectIOHelper>(
            args, m_dioService, userCTXFactory);
    }

    if (sh_name == "AmazonS3")
        return std::make_shared<S3Helper>(args, m_s3Service);

    return {};
}

} // namespace helpers
} // namespace one
