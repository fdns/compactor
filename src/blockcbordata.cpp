/*
 * Copyright 2016-2018 Internet Corporation for Assigned Names and Numbers.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, you can obtain one at https://mozilla.org/MPL/2.0/.
 */

/*
 * Developed by Sinodun IT (www.sinodun.com)
 */

#include "blockcbordata.hpp"

namespace block_cbor {

    void IndexVectorItem::readCbor(CborBaseDecoder& dec, const FileVersionFields&)
    {
        try
        {
            bool indef;
            uint64_t n_elems = dec.readArrayHeader(indef);
            if ( !indef )
                vec.reserve(n_elems);
            while ( indef || n_elems-- > 0 )
            {
                if ( indef && dec.type() == CborBaseDecoder::TYPE_BREAK )
                {
                    dec.readBreak();
                    break;
                }

                vec.push_back(static_cast<index_t>(dec.read_unsigned()));
            }
        }
        catch (const std::logic_error& e)
        {
            throw cbor_file_format_error("Unexpected CBOR item reading index");
        }
    }

    void IndexVectorItem::writeCbor(CborBaseEncoder& enc)
    {
        enc.writeArrayHeader(vec.size());
        for ( auto& i : vec )
            enc.write(i);
    }

    void ByteStringItem::readCbor(CborBaseDecoder& dec, const FileVersionFields&)
    {
        try
        {
            str = dec.read_binary();
        }
        catch (const std::logic_error& e)
        {
            throw cbor_file_format_error("Unexpected CBOR item reading string");
        }
    }

    void ByteStringItem::writeCbor(CborBaseEncoder& enc)
    {
        enc.write(str);
    }

    void IPAddressItem::readCbor(CborBaseDecoder& dec, const FileVersionFields&)
    {
        try
        {
            addr = IPAddress(dec.read_binary());
        }
        catch (const std::logic_error& e)
        {
            throw cbor_file_format_error("Unexpected CBOR item reading IP address");
        }
    }

    void IPAddressItem::writeCbor(CborBaseEncoder& enc)
    {
        enc.write(addr.asNetworkBinary());
    }

    void ClassType::readCbor(CborBaseDecoder& dec, const FileVersionFields& fields)
    {
        try
        {
            bool indef;
            uint64_t n_elems = dec.readMapHeader(indef);
            while ( indef || n_elems-- > 0 )
            {
                if ( indef && dec.type() == CborBaseDecoder::TYPE_BREAK )
                {
                    dec.readBreak();
                    break;
                }

                switch(fields.class_type_field(dec.read_unsigned()))
                {
                case ClassTypeField::type_id:
                    qtype = CaptureDNS::QueryType(dec.read_unsigned());
                    break;

                case ClassTypeField::class_id:
                    qclass = CaptureDNS::QueryClass(dec.read_unsigned());
                    break;

                default:
                    // Unknown item, skip.
                    dec.skip();
                    break;
                }
            }
        }
        catch (const std::logic_error& e)
        {
            throw cbor_file_format_error("Unexpected CBOR item reading ClassType");
        }
    }

    void ClassType::writeCbor(CborBaseEncoder& enc)
    {
        constexpr unsigned type_index = find_class_type_index(ClassTypeField::type_id);
        constexpr unsigned class_index = find_class_type_index(ClassTypeField::class_id);

        enc.writeMapHeader(2);
        enc.write(type_index);
        enc.write(qtype);
        enc.write(class_index);
        enc.write(qclass);
    }

    std::size_t hash_value(const ClassType &ct)
    {
        std::size_t seed = boost::hash_value(ct.qclass);
        boost::hash_combine(seed, ct.qtype);
        return seed;
    }

    void Question::readCbor(CborBaseDecoder& dec, const FileVersionFields& fields)
    {
        try
        {
            bool indef;
            uint64_t n_elems = dec.readMapHeader(indef);
            while ( indef || n_elems-- > 0 )
            {
                if ( indef && dec.type() == CborBaseDecoder::TYPE_BREAK )
                {
                    dec.readBreak();
                    break;
                }

                switch(fields.question_field(dec.read_unsigned()))
                {
                case QuestionField::name_index:
                    qname = dec.read_unsigned();
                    break;

                case QuestionField::classtype_index:
                    classtype = dec.read_unsigned();
                    break;

                default:
                    // Unknown item, skip.
                    dec.skip();
                    break;
                }
            }
        }
        catch (const std::logic_error& e)
        {
            throw cbor_file_format_error("Unexpected CBOR item reading Question");
        }
    }

    void Question::writeCbor(CborBaseEncoder& enc)
    {
        constexpr unsigned qname_index = find_question_index(QuestionField::name_index);
        constexpr unsigned classtype_index = find_question_index(QuestionField::classtype_index);

        enc.writeMapHeader(2);
        enc.write(qname_index);
        enc.write(qname);
        enc.write(classtype_index);
        enc.write(classtype);
    }

    std::size_t hash_value(const Question& q)
    {
        std::size_t seed = boost::hash_value(q.qname);
        boost::hash_combine(seed, q.classtype);
        return seed;
    }

    void ResourceRecord::readCbor(CborBaseDecoder& dec, const FileVersionFields& fields)
    {
        try
        {
            bool indef;
            uint64_t n_elems = dec.readMapHeader(indef);
            while ( indef || n_elems-- > 0 )
            {
                if ( indef && dec.type() == CborBaseDecoder::TYPE_BREAK )
                {
                    dec.readBreak();
                    break;
                }

                switch(fields.rr_field(dec.read_unsigned()))
                {
                case RRField::name_index:
                    name = dec.read_unsigned();
                    break;

                case RRField::classtype_index:
                    classtype = dec.read_unsigned();
                    break;

                case RRField::ttl:
                    ttl = dec.read_unsigned();
                    break;

                case RRField::rdata_index:
                    rdata = dec.read_unsigned();
                    break;

                default:
                    // Unknown item, skip.
                    dec.skip();
                    break;
                }
            }
        }
        catch (const std::logic_error& e)
        {
            throw cbor_file_format_error("Unexpected CBOR item reading ResourceRecord");
        }
    }

    void ResourceRecord::writeCbor(CborBaseEncoder& enc)
    {
        constexpr unsigned name_index = find_rr_index(RRField::name_index);
        constexpr unsigned classtype_index = find_rr_index(RRField::classtype_index);
        constexpr unsigned ttl_index = find_rr_index(RRField::ttl);
        constexpr unsigned rdata_index = find_rr_index(RRField::rdata_index);

        enc.writeMapHeader(4);
        enc.write(name_index);
        enc.write(name);
        enc.write(classtype_index);
        enc.write(classtype);
        enc.write(ttl_index);
        enc.write(ttl);
        enc.write(rdata_index);
        enc.write(rdata);
    }

    std::size_t hash_value(const ResourceRecord& rr)
    {
        std::size_t seed = boost::hash_value(rr.name);
        boost::hash_combine(seed, rr.classtype);
        boost::hash_combine(seed, rr.ttl);
        boost::hash_combine(seed, rr.rdata);
        return seed;
    }

    void QuerySignature::readCbor(CborBaseDecoder& dec, const FileVersionFields& fields)
    {
        try
        {
            bool indef;
            uint64_t n_elems = dec.readMapHeader(indef);
            while ( indef || n_elems-- > 0 )
            {
                if ( indef && dec.type() == CborBaseDecoder::TYPE_BREAK )
                {
                    dec.readBreak();
                    break;
                }

                switch(fields.query_signature_field(dec.read_unsigned()))
                {
                case QuerySignatureField::server_address_index:
                    server_address = dec.read_unsigned();
                    break;

                case QuerySignatureField::server_port:
                    server_port = dec.read_unsigned();
                    break;

                case QuerySignatureField::transport_flags:
                    transport_flags = dec.read_unsigned();
                    break;

                case QuerySignatureField::qr_dns_flags:
                    dns_flags = dec.read_unsigned();
                    break;

                case QuerySignatureField::qr_sig_flags:
                    qr_flags = dec.read_unsigned();
                    break;

                case QuerySignatureField::query_qd_count:
                    qdcount = dec.read_unsigned();
                    break;

                case QuerySignatureField::query_classtype_index:
                    query_classtype = dec.read_unsigned();
                    break;

                case QuerySignatureField::query_rcode:
                    query_rcode = dec.read_unsigned();
                    break;

                case QuerySignatureField::query_opcode:
                    query_opcode = dec.read_unsigned();
                    break;

                case QuerySignatureField::query_an_count:
                    query_ancount = dec.read_unsigned();
                    break;

                case QuerySignatureField::query_ar_count:
                    query_arcount = dec.read_unsigned();
                    break;

                case QuerySignatureField::query_ns_count:
                    query_nscount = dec.read_unsigned();
                    break;

                case QuerySignatureField::edns_version:
                    query_edns_version = dec.read_unsigned();
                    break;

                case QuerySignatureField::udp_buf_size:
                    query_edns_payload_size = dec.read_unsigned();
                    break;

                case QuerySignatureField::opt_rdata_index:
                    query_opt_rdata = dec.read_unsigned();
                    break;

                case QuerySignatureField::response_rcode:
                    response_rcode = dec.read_unsigned();
                    break;

                default:
                    // Unknown item, skip.
                    dec.skip();
                    break;
                }
            }
        }
        catch (const std::logic_error& e)
        {
            throw cbor_file_format_error("Unexpected CBOR item reading QuerySignature");
        }
    }

    void QuerySignature::writeCbor(CborBaseEncoder& enc)
    {
        constexpr unsigned server_address_index = find_query_signature_index(QuerySignatureField::server_address_index);
        constexpr unsigned server_port_index = find_query_signature_index(QuerySignatureField::server_port);
        constexpr unsigned transport_flags_index = find_query_signature_index(QuerySignatureField::transport_flags);
        constexpr unsigned qr_sig_flags_index = find_query_signature_index(QuerySignatureField::qr_sig_flags);
        constexpr unsigned query_opcode_index = find_query_signature_index(QuerySignatureField::query_opcode);
        constexpr unsigned qr_dns_flags_index = find_query_signature_index(QuerySignatureField::qr_dns_flags);
        constexpr unsigned query_rcode_index = find_query_signature_index(QuerySignatureField::query_rcode);
        constexpr unsigned query_classtype_index = find_query_signature_index(QuerySignatureField::query_classtype_index);
        constexpr unsigned query_qd_index = find_query_signature_index(QuerySignatureField::query_qd_count);
        constexpr unsigned query_an_index = find_query_signature_index(QuerySignatureField::query_an_count);
        constexpr unsigned query_ar_index = find_query_signature_index(QuerySignatureField::query_ar_count);
        constexpr unsigned query_ns_index = find_query_signature_index(QuerySignatureField::query_ns_count);
        constexpr unsigned edns_version_index = find_query_signature_index(QuerySignatureField::edns_version);
        constexpr unsigned udp_buf_size_index = find_query_signature_index(QuerySignatureField::udp_buf_size);
        constexpr unsigned opt_rdata_index = find_query_signature_index(QuerySignatureField::opt_rdata_index);
        constexpr unsigned response_rcode_index = find_query_signature_index(QuerySignatureField::response_rcode);

        enc.writeMapHeader();
        enc.write(server_address_index);
        enc.write(server_address);
        enc.write(server_port_index);
        enc.write(server_port);
        enc.write(transport_flags_index);
        enc.write(transport_flags);
        enc.write(qr_dns_flags_index);
        enc.write(dns_flags);
        enc.write(qr_sig_flags_index);
        enc.write(qr_flags);
        enc.write(query_qd_index);
        enc.write(qdcount);
        if ( qr_flags & QR_HAS_QUESTION )
        {
            enc.write(query_classtype_index);
            enc.write(query_classtype);
        }
        if ( qr_flags & QUERY_ONLY )
        {
            enc.write(query_rcode_index);
            enc.write(query_rcode);
            enc.write(query_opcode_index);
            enc.write(query_opcode);
            enc.write(query_an_index);
            enc.write(query_ancount);
            enc.write(query_ar_index);
            enc.write(query_arcount);
            enc.write(query_ns_index);
            enc.write(query_nscount);

            if ( qr_flags & QUERY_HAS_OPT )
            {
                enc.write(edns_version_index);
                enc.write(query_edns_version);
                enc.write(udp_buf_size_index);
                enc.write(query_edns_payload_size);
                enc.write(opt_rdata_index);
                enc.write(query_opt_rdata);
            }
        }
        if ( qr_flags & RESPONSE_ONLY )
        {
            enc.write(response_rcode_index);
            enc.write(response_rcode);
        }
        enc.writeBreak();
    }

    std::size_t hash_value(const QuerySignature& qs)
    {
        std::size_t seed = boost::hash_value(qs.server_address);
        boost::hash_combine(seed, qs.server_port);
        boost::hash_combine(seed, qs.transport_flags);
        boost::hash_combine(seed, qs.dns_flags);
        boost::hash_combine(seed, qs.qr_flags);
        boost::hash_combine(seed, qs.qdcount);
        if ( qs.qr_flags & QR_HAS_QUESTION )
            boost::hash_combine(seed, qs.query_classtype);
        if ( qs.qr_flags & QUERY_ONLY )
        {
            boost::hash_combine(seed, qs.query_rcode);
            boost::hash_combine(seed, qs.query_opcode);
            boost::hash_combine(seed, qs.query_ancount);
            boost::hash_combine(seed, qs.query_nscount);
            boost::hash_combine(seed, qs.query_arcount);
        }
        if ( qs.qr_flags & RESPONSE_ONLY )
            boost::hash_combine(seed, qs.response_rcode);
        if ( qs.qr_flags & QUERY_HAS_OPT )
        {
            boost::hash_combine(seed, qs.query_edns_version);
            boost::hash_combine(seed, qs.query_edns_payload_size);
            boost::hash_combine(seed, qs.query_opt_rdata);
        }
        return seed;
    }

    namespace {
        /**
         * \brief Read a `QueryResponseExtraInfo` from CBOR.
         *
         * \param dec    CBOR stream to read from.
         * \param fields translate map keys to internal values.
         * \throws cbor_file_format_error on unexpected CBOR content.
         * \throws cbor_decode_error on malformed CBOR items.
         * \throws cbor_end_of_input on end of CBOR file.
         */
        std::unique_ptr<QueryResponseExtraInfo> readExtraInfo(CborBaseDecoder& dec, const FileVersionFields& fields)
        {
            std::unique_ptr<QueryResponseExtraInfo> res = make_unique<QueryResponseExtraInfo>();
            bool indef;
            uint64_t n_elems = dec.readMapHeader(indef);
            while ( indef || n_elems-- > 0 )
            {
                if ( indef && dec.type() == CborBaseDecoder::TYPE_BREAK )
                {
                    dec.readBreak();
                    break;
                }

                switch(fields.query_response_extended_field(dec.read_unsigned()))
                {
                case QueryResponseExtendedField::question_index:
                    res->questions_list = dec.read_unsigned();
                    break;

                case QueryResponseExtendedField::answer_index:
                    res->answers_list = dec.read_unsigned();
                    break;

                case QueryResponseExtendedField::authority_index:
                    res->authority_list = dec.read_unsigned();
                    break;

                case QueryResponseExtendedField::additional_index:
                    res->additional_list = dec.read_unsigned();
                    break;

                default:
                    dec.skip();
                    break;
                }
            }
            return res;
        }

        /**
         * \brief Write `QueryResponseExtraInfo` contents to CBOR.
         *
         * \param enc CBOR stream to write to.
         * \param id  the CBOR item identifier.
         * \param ei  the extra info to write.
         */
        void writeExtraInfo(CborBaseEncoder& enc, int id,
                            const QueryResponseExtraInfo& ei)
        {
            constexpr unsigned questions_index = find_query_response_extended_index(QueryResponseExtendedField::question_index);
            constexpr unsigned answer_index = find_query_response_extended_index(QueryResponseExtendedField::answer_index);
            constexpr unsigned authority_index = find_query_response_extended_index(QueryResponseExtendedField::authority_index);
            constexpr unsigned additional_index = find_query_response_extended_index(QueryResponseExtendedField::additional_index);

            enc.write(id);
            enc.writeMapHeader();
            if ( ei.questions_list != 0 )
            {
                enc.write(questions_index);
                enc.write(ei.questions_list);
            }
            if ( ei.answers_list != 0 )
            {
                enc.write(answer_index);
                enc.write(ei.answers_list);
            }
            if ( ei.authority_list != 0 )
            {
                enc.write(authority_index);
                enc.write(ei.authority_list);
            }
            if ( ei.additional_list != 0 )
            {
                enc.write(additional_index);
                enc.write(ei.additional_list);
            }
            enc.writeBreak();
        }
    }

    void QueryResponseItem::clear()
    {
        qr_flags = client_port = hoplimit = id = query_size = response_size = 0;
        tstamp = std::chrono::system_clock::time_point(std::chrono::microseconds(0));
        response_delay = std::chrono::microseconds(0);
        client_address = qname = signature = 0;
        query_extra_info.release();
        response_extra_info.release();
    }

    void QueryResponseItem::readCbor(CborBaseDecoder& dec,
                                     const std::chrono::system_clock::time_point& earliest_time,
                                     const FileVersionFields& fields)
    {
        try
        {
            qr_flags = 0;

            bool indef;
            uint64_t n_elems = dec.readMapHeader(indef);
            while ( indef || n_elems-- > 0 )
            {
                if ( indef && dec.type() == CborBaseDecoder::TYPE_BREAK )
                {
                    dec.readBreak();
                    break;
                }

                switch(fields.query_response_field(dec.read_unsigned()))
                {
                case QueryResponseField::time_useconds:
                    tstamp = earliest_time + std::chrono::microseconds(dec.read_signed());
                    break;

                case QueryResponseField::client_address_index:
                    client_address = dec.read_unsigned();
                    break;

                case QueryResponseField::client_port:
                    client_port = dec.read_unsigned();
                    break;

                case QueryResponseField::transaction_id:
                    id = dec.read_unsigned();
                    break;

                case QueryResponseField::query_signature_index:
                    signature = dec.read_unsigned();
                    break;

                case QueryResponseField::client_hoplimit:
                    hoplimit = dec.read_unsigned();
                    qr_flags |= QUERY_ONLY;
                    break;

                case QueryResponseField::delay_useconds:
                    response_delay = std::chrono::microseconds(dec.read_signed());
                    qr_flags |= QUERY_AND_RESPONSE;
                    break;

                case QueryResponseField::query_name_index:
                    qname = dec.read_unsigned();
                    qr_flags |= QR_HAS_QUESTION;
                    break;

                case QueryResponseField::query_size:
                    query_size = dec.read_unsigned();
                    qr_flags |= QUERY_ONLY;
                    break;

                case QueryResponseField::response_size:
                    response_size = dec.read_unsigned();
                    qr_flags |= RESPONSE_ONLY;
                    break;

                case QueryResponseField::query_extended:
                    query_extra_info = readExtraInfo(dec, fields);
                    break;

                case QueryResponseField::response_extended:
                    response_extra_info = readExtraInfo(dec, fields);
                    break;

                default:
                    // Unknown item, skip.
                    dec.skip();
                    break;
                }
            }
        }
        catch (const std::logic_error& e)
        {
            throw cbor_file_format_error("Unexpected CBOR item reading QueryResponseItem");
        }
    }

    void QueryResponseItem::writeCbor(CborBaseEncoder& enc,
                                      const std::chrono::system_clock::time_point& earliest_time)
    {
        constexpr unsigned time_index = find_query_response_index(QueryResponseField::time_useconds);
        constexpr unsigned client_address_index = find_query_response_index(QueryResponseField::client_address_index);
        constexpr unsigned client_port_index = find_query_response_index(QueryResponseField::client_port);
        constexpr unsigned transaction_id_index = find_query_response_index(QueryResponseField::transaction_id);
        constexpr unsigned query_signature_index = find_query_response_index(QueryResponseField::query_signature_index);
        constexpr unsigned client_hoplimit_index = find_query_response_index(QueryResponseField::client_hoplimit);
        constexpr unsigned delay_index = find_query_response_index(QueryResponseField::delay_useconds);
        constexpr unsigned query_name_index = find_query_response_index(QueryResponseField::query_name_index);
        constexpr unsigned query_size_index = find_query_response_index(QueryResponseField::query_size);
        constexpr unsigned response_size_index = find_query_response_index(QueryResponseField::response_size);
        constexpr unsigned query_extended_index = find_query_response_index(QueryResponseField::query_extended);
        constexpr unsigned response_extended_index = find_query_response_index(QueryResponseField::response_extended);

        enc.writeMapHeader();
        enc.write(time_index);
        enc.write(std::chrono::duration_cast<std::chrono::microseconds>(tstamp - earliest_time));
        enc.write(client_address_index);
        enc.write(client_address);
        enc.write(client_port_index);
        enc.write(client_port);
        enc.write(transaction_id_index);
        enc.write(id);
        enc.write(query_signature_index);
        enc.write(signature);

        if ( qr_flags & QUERY_ONLY )
        {
            enc.write(client_hoplimit_index);
            enc.write(hoplimit);
        }

        if ( ( qr_flags & QUERY_AND_RESPONSE ) == QUERY_AND_RESPONSE )
        {
            enc.write(delay_index);
            enc.write(response_delay);
        }

        if ( qr_flags & QR_HAS_QUESTION )
        {
            enc.write(query_name_index);
            enc.write(qname);
        }

        if ( qr_flags & QUERY_ONLY )
        {
            enc.write(query_size_index);
            enc.write(query_size);
        }

        if ( qr_flags & RESPONSE_ONLY )
        {
            enc.write(response_size_index);
            enc.write(response_size);
        }

        if ( query_extra_info )
            writeExtraInfo(enc, query_extended_index, *query_extra_info);

        if ( response_extra_info )
            writeExtraInfo(enc, response_extended_index, *response_extra_info);

        enc.writeBreak();
    }

    std::size_t hash_value(const AddressEventItem& aei)
    {
        std::size_t seed = boost::hash_value(aei.type);
        boost::hash_combine(seed, aei.code);
        boost::hash_combine(seed, aei.address);
        return seed;
    }

    void AddressEventCount::readCbor(CborBaseDecoder& dec, const FileVersionFields& fields)
    {
        try
        {
            // No necessarily present, default is 0.
            aei.code = 0;

            bool indef;
            uint64_t n_elems = dec.readMapHeader(indef);
            while ( indef || n_elems-- > 0 )
            {
                if ( indef && dec.type() == CborBaseDecoder::TYPE_BREAK )
                {
                    dec.readBreak();
                    break;
                }

                switch(fields.address_event_count_field(dec.read_unsigned()))
                {
                case AddressEventCountField::ae_type:
                    aei.type = static_cast<AddressEvent::EventType>(dec.read_unsigned());
                    break;

                case AddressEventCountField::ae_code:
                    aei.code = dec.read_unsigned();
                    break;

                case AddressEventCountField::ae_address_index:
                    aei.address = dec.read_unsigned();
                    break;

                case AddressEventCountField::ae_count:
                    count = dec.read_unsigned();
                    break;

                default:
                    // Unknown item, skip.
                    dec.skip();
                    break;
                }
            }
        }
        catch (const std::logic_error& e)
        {
            throw cbor_file_format_error("Unexpected CBOR item reading AddressEvent");
        }
    }

    void AddressEventCount::writeCbor(CborBaseEncoder& enc)
    {
        constexpr unsigned type_index = find_address_event_count_index(AddressEventCountField::ae_type);
        constexpr unsigned code_index = find_address_event_count_index(AddressEventCountField::ae_code);
        constexpr unsigned address_index = find_address_event_count_index(AddressEventCountField::ae_address_index);
        constexpr unsigned count_index = find_address_event_count_index(AddressEventCountField::ae_count);

        enc.writeMapHeader();
        enc.write(type_index);
        enc.write(aei.type);
        if ( aei.code != 0 )
        {
            enc.write(code_index);
            enc.write(aei.code);
        }
        enc.write(address_index);
        enc.write(aei.address);
        enc.write(count_index);
        enc.write(count);
        enc.writeBreak();
    }

    void BlockData::readCbor(CborBaseDecoder& dec, const FileVersionFields& fields)
    {
        bool indef;
        uint64_t n_elems = dec.readMapHeader(indef);
        while ( indef || n_elems-- > 0 )
        {
            if ( indef && dec.type() == CborBaseDecoder::TYPE_BREAK )
            {
                dec.readBreak();
                break;
            }

            switch(fields.block_field(dec.read_unsigned()))
            {
            case BlockField::preamble:
                readBlockPreamble(dec, fields);
                break;

            case BlockField::tables:
                readHeaders(dec, fields);
                break;

            case BlockField::statistics:
                readStats(dec, fields);
                break;

            case BlockField::queries:
                readItems(dec, fields);
                break;

            case BlockField::address_event_counts:
                readAddressEventCounts(dec, fields);
                break;

            default:
                dec.skip();
                break;
            }
        }
    }

    void BlockData::readBlockPreamble(CborBaseDecoder& dec, const FileVersionFields& fields)
    {
        bool indef;
        uint64_t n_elems = dec.readMapHeader(indef);
        while ( indef || n_elems-- > 0 )
        {
            if ( indef && dec.type() == CborBaseDecoder::TYPE_BREAK )
            {
                dec.readBreak();
                break;
            }

            switch(fields.block_preamble_field(dec.read_unsigned()))
            {
            case BlockPreambleField::earliest_time:
                earliest_time = dec.read_time();
                break;

            default:
                dec.skip();
                break;
            }
        }
    }

    void BlockData::readHeaders(CborBaseDecoder& dec, const FileVersionFields& fields)
    {
        bool indef;
        uint64_t n_elems = dec.readMapHeader(indef);
        while ( indef || n_elems-- > 0 )
        {
            if ( indef && dec.type() == CborBaseDecoder::TYPE_BREAK )
            {
                dec.readBreak();
                break;
            }

            switch(fields.block_tables_field(dec.read_unsigned()))
            {
            case BlockTablesField::ip_address:
                ip_addresses.readCbor(dec, fields);
                break;

            case BlockTablesField::classtype:
                class_types.readCbor(dec, fields);
                break;

            case BlockTablesField::name_rdata:
                names_rdatas.readCbor(dec, fields);
                break;

            case BlockTablesField::query_signature:
                query_signatures.readCbor(dec, fields);
                break;

            case BlockTablesField::question_list:
                questions_lists.readCbor(dec, fields);
                break;

            case BlockTablesField::question_rr:
                questions.readCbor(dec, fields);
                break;

            case BlockTablesField::rr_list:
                rrs_lists.readCbor(dec, fields);
                break;

            case BlockTablesField::rr:
                resource_records.readCbor(dec, fields);
                break;

            default:
                dec.skip();
                break;
            }
        }
    }

    void BlockData::readItems(CborBaseDecoder& dec, const FileVersionFields& fields)
    {
        bool indef;
        uint64_t n_elems = dec.readArrayHeader(indef);
        if ( !indef )
            query_response_items.reserve(n_elems);
        while ( indef || n_elems-- > 0 )
        {
            if ( indef && dec.type() == CborBaseDecoder::TYPE_BREAK )
            {
                dec.readBreak();
                break;
            }

            QueryResponseItem qri;
            qri.readCbor(dec, earliest_time, fields);
            query_response_items.push_back(std::move(qri));
        }
    }

    void BlockData::readStats(CborBaseDecoder& dec, const FileVersionFields& fields)
    {
        start_packet_statistics = {};

        bool indef;
        uint64_t n_elems = dec.readMapHeader(indef);
        while ( indef || n_elems-- > 0 )
        {
            if ( indef && dec.type() == CborBaseDecoder::TYPE_BREAK )
            {
                dec.readBreak();
                break;
            }

            switch(fields.block_statistics_field(dec.read_unsigned()))
            {
            case BlockStatisticsField::completely_malformed_packets:
                last_packet_statistics.malformed_packet_count += dec.read_unsigned();
                break;

            case BlockStatisticsField::compactor_non_dns_packets:
                last_packet_statistics.unhandled_packet_count += dec.read_unsigned();
                break;

            case BlockStatisticsField::compactor_out_of_order_packets:
                last_packet_statistics.out_of_order_packet_count += dec.read_unsigned();
                break;

            case BlockStatisticsField::total_packets:
                last_packet_statistics.raw_packet_count += dec.read_unsigned();
                break;

            case BlockStatisticsField::compactor_missing_pairs:
                last_packet_statistics.output_cbor_drop_count += dec.read_unsigned();
                break;

            case BlockStatisticsField::unmatched_queries:
                last_packet_statistics.query_without_response_count += dec.read_unsigned();
                break;

            case BlockStatisticsField::unmatched_responses:
                last_packet_statistics.response_without_query_count += dec.read_unsigned();
                break;

            case BlockStatisticsField::total_pairs:
                last_packet_statistics.qr_pair_count += dec.read_unsigned();
                break;

            case BlockStatisticsField::compactor_missing_packets:
                last_packet_statistics.output_raw_pcap_drop_count += dec.read_unsigned();
                break;

            case BlockStatisticsField::compactor_missing_non_dns:
                last_packet_statistics.output_ignored_pcap_drop_count += dec.read_unsigned();
                break;

            default:
                dec.skip();
                break;
            }
        }
    }

    void BlockData::readAddressEventCounts(CborBaseDecoder& dec, const FileVersionFields& fields)
    {
        bool indef;
        uint64_t n_elems = dec.readArrayHeader(indef);
        while ( indef || n_elems-- > 0 )
        {
            if ( indef && dec.type() == CborBaseDecoder::TYPE_BREAK )
            {
                dec.readBreak();
                break;
            }

            AddressEventCount aec = {};
            aec.readCbor(dec, fields);
            address_event_counts[aec.aei] = aec.count;
        }
    }

    void BlockData::writeCbor(CborBaseEncoder& enc)
    {
        constexpr unsigned preamble_index = find_block_index(BlockField::preamble);
        constexpr unsigned statistics_index = find_block_index(BlockField::statistics);
        constexpr unsigned tables_index = find_block_index(BlockField::tables);
        constexpr unsigned queries_index = find_block_index(BlockField::queries);
        constexpr unsigned aec_index = find_block_index(BlockField::address_event_counts);
        constexpr unsigned earliest_time_index = find_block_preamble_index(BlockPreambleField::earliest_time);

        // Block header.
        enc.writeMapHeader();

        // Block preamble.
        enc.write(preamble_index);
        enc.writeMapHeader(1);
        enc.write(earliest_time_index);
        enc.write(earliest_time);

        // Statistics.
        enc.write(statistics_index);
        writeStats(enc);

        // Header tables.
        enc.write(tables_index);
        writeHeaders(enc);

        // Block items.
        enc.write(queries_index);
        writeItems(enc);

        // Address event items.
        enc.write(aec_index);
        writeAddressEventCounts(enc);

        // Block terminator.
        enc.writeBreak();
    }

    void BlockData::writeHeaders(CborBaseEncoder& enc)
    {
        constexpr unsigned ipaddress_index = find_block_tables_index(BlockTablesField::ip_address);
        constexpr unsigned classtype_index = find_block_tables_index(BlockTablesField::classtype);
        constexpr unsigned name_rdata_index = find_block_tables_index(BlockTablesField::name_rdata);
        constexpr unsigned query_signature_index = find_block_tables_index(BlockTablesField::query_signature);
        constexpr unsigned question_list_index = find_block_tables_index(BlockTablesField::question_list);
        constexpr unsigned question_rr_index = find_block_tables_index(BlockTablesField::question_rr);
        constexpr unsigned rr_list_index = find_block_tables_index(BlockTablesField::rr_list);
        constexpr unsigned rr_index = find_block_tables_index(BlockTablesField::rr);

        enc.writeMapHeader();
        enc.write(ipaddress_index);
        ip_addresses.writeCbor(enc);
        enc.write(classtype_index);
        class_types.writeCbor(enc);
        enc.write(name_rdata_index);
        names_rdatas.writeCbor(enc);
        enc.write(query_signature_index);
        query_signatures.writeCbor(enc);
        enc.write(question_list_index);
        questions_lists.writeCbor(enc);
        enc.write(question_rr_index);
        questions.writeCbor(enc);
        enc.write(rr_list_index);
        rrs_lists.writeCbor(enc);
        enc.write(rr_index);
        resource_records.writeCbor(enc);
        enc.writeBreak();
    }

    void BlockData::writeItems(CborBaseEncoder& enc)
    {
        enc.writeArrayHeader(query_response_items.size());
        for ( auto& qri : query_response_items )
            qri.writeCbor(enc, earliest_time);
    }

    void BlockData::writeStats(CborBaseEncoder& enc)
    {
        constexpr unsigned total_packets_index = find_block_statistics_index(BlockStatisticsField::total_packets);
        constexpr unsigned total_pairs_index = find_block_statistics_index(BlockStatisticsField::total_pairs);
        constexpr unsigned unmatched_queries_index = find_block_statistics_index(BlockStatisticsField::unmatched_queries);
        constexpr unsigned unmatched_responses_index = find_block_statistics_index(BlockStatisticsField::unmatched_responses);
        constexpr unsigned malformed_packets_index = find_block_statistics_index(BlockStatisticsField::completely_malformed_packets);
        constexpr unsigned non_dns_packets_index = find_block_statistics_index(BlockStatisticsField::compactor_non_dns_packets);
        constexpr unsigned out_of_order_packets_index = find_block_statistics_index(BlockStatisticsField::compactor_out_of_order_packets);
        constexpr unsigned missing_pairs_index = find_block_statistics_index(BlockStatisticsField::compactor_missing_pairs);
        constexpr unsigned missing_packets_index = find_block_statistics_index(BlockStatisticsField::compactor_missing_packets);
        constexpr unsigned missing_non_dns_index = find_block_statistics_index(BlockStatisticsField::compactor_missing_non_dns);

        enc.writeMapHeader();
        enc.write(total_packets_index);
        enc.write(last_packet_statistics.raw_packet_count - start_packet_statistics.raw_packet_count);
        enc.write(total_pairs_index);
        enc.write(last_packet_statistics.qr_pair_count - start_packet_statistics.qr_pair_count);
        enc.write(unmatched_queries_index);
        enc.write(last_packet_statistics.query_without_response_count - start_packet_statistics.query_without_response_count);
        enc.write(unmatched_responses_index);
        enc.write(last_packet_statistics.response_without_query_count - start_packet_statistics.response_without_query_count);
        enc.write(malformed_packets_index);
        enc.write(last_packet_statistics.malformed_packet_count - start_packet_statistics.malformed_packet_count);
        enc.write(non_dns_packets_index);
        enc.write(last_packet_statistics.unhandled_packet_count - start_packet_statistics.unhandled_packet_count);
        enc.write(out_of_order_packets_index);
        enc.write(last_packet_statistics.out_of_order_packet_count - start_packet_statistics.out_of_order_packet_count);
        enc.write(missing_pairs_index);
        enc.write(last_packet_statistics.output_cbor_drop_count - start_packet_statistics.output_cbor_drop_count);
        enc.write(missing_packets_index);
        enc.write(last_packet_statistics.output_raw_pcap_drop_count - start_packet_statistics.output_raw_pcap_drop_count);
        enc.write(missing_non_dns_index);
        enc.write(last_packet_statistics.output_ignored_pcap_drop_count - start_packet_statistics.output_ignored_pcap_drop_count);
        enc.writeBreak();
    }

    void BlockData::writeAddressEventCounts(CborBaseEncoder& enc)
    {
        enc.writeArrayHeader(address_event_counts.size());
        for ( auto& aeci : address_event_counts )
        {
            AddressEventCount aec;
            aec.aei = aeci.first;
            aec.count = aeci.second;
            aec.writeCbor(enc);
        }
    }

}
