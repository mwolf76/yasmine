/**
 * @file sat/helpers.hh
 *
 * This header file contains the declarations required to implement a
 * range of helpers needed during the compilation to SAT.
 *
 * Copyright (C) 2012 Marco Pensallorto < marco AT pensallorto DOT gmail DOT com >
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA
 *
 **/

#ifndef SAT_HELPERS
#define SAT_HELPERS

#include <sat/typedefs.hh>
#include <dd/dd_walker.hh>

#include <model/compiler/unit.hh>

#include <boost/filesystem.hpp>
#include <boost/thread/mutex.hpp>

class Engine;

typedef class InlinedOperatorLoader* InlinedOperatorLoader_ptr;
typedef boost::unordered_map<InlinedOperatorSignature, InlinedOperatorLoader_ptr,
                             InlinedOperatorSignatureHash,
                             InlinedOperatorSignatureEq> InlinedOperatorLoaderMap;


class InlinedOperatorLoader {
public:
    InlinedOperatorLoader(const boost::filesystem::path& filepath);
    ~InlinedOperatorLoader();

    inline const InlinedOperatorSignature& ios() const
    { return f_ios; }

    // synchronized
    const LitsVector& clauses();

private:
    boost::mutex f_loading_mutex;
    LitsVector f_clauses;

    boost::filesystem::path f_fullpath;
    InlinedOperatorSignature f_ios;
};

typedef class InlinedOperatorMgr *InlinedOperatorMgr_ptr;
class InlinedOperatorMgr  {

public:
    static InlinedOperatorMgr& INSTANCE() {
        if (! f_instance)
            f_instance = new InlinedOperatorMgr();

        return (*f_instance);
    }

    InlinedOperatorLoader& require(const InlinedOperatorSignature& ios);

    inline const InlinedOperatorLoaderMap& loaders() const
    { return f_loaders; }

protected:
    InlinedOperatorMgr();
    ~InlinedOperatorMgr();

private:
    static InlinedOperatorMgr_ptr f_instance;
    std::string f_builtin_microcode_path;

    InlinedOperatorLoaderMap f_loaders;
};


class CNFOperatorInliner {
public:
    CNFOperatorInliner(Engine& sat, step_t time, group_t group = MAINGROUP)
        : f_sat(sat)
        , f_time(time)
        , f_group(group)
    {}

    ~CNFOperatorInliner()
    {}

    inline void operator() (const InlinedOperatorDescriptor& md)
    {
        InlinedOperatorMgr& mm
            (InlinedOperatorMgr::INSTANCE());

        InlinedOperatorSignature ios
            (md.ios());
        InlinedOperatorLoader& loader
            (mm.require(ios));

        inject(md, loader.clauses());
    }

private:
    void inject(const InlinedOperatorDescriptor& md,
                const LitsVector& clauses);

    Engine& f_sat;
    step_t f_time;
    group_t f_group;
};

class CNFBinarySelectionInliner {
public:
    CNFBinarySelectionInliner(Engine& sat, step_t time, group_t group = MAINGROUP)
        : f_sat(sat)
        , f_time(time)
        , f_group(group)
    {}

    ~CNFBinarySelectionInliner()
    {}

    inline void operator() (const BinarySelectionDescriptor& md)
    { inject(md); }

private:
    void inject(const BinarySelectionDescriptor& md);

    Engine& f_sat;
    step_t f_time;
    group_t f_group;
};

class CNFMultiwaySelectionInliner {
public:
    CNFMultiwaySelectionInliner(Engine& sat, step_t time, group_t group = MAINGROUP)
        : f_sat(sat)
        , f_time(time)
        , f_group(group)
    {}

    ~CNFMultiwaySelectionInliner()
    {}

    inline void operator() (const MultiwaySelectionDescriptor& md)
    { inject(md); }

private:
    void inject(const MultiwaySelectionDescriptor& md);

    Engine& f_sat;
    step_t f_time;
    group_t f_group;
};

#endif /* SAT_HELPERS */
