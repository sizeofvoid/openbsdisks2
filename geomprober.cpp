/*
    Copyright 2016 Gleb Popov <6yearold@gmail.com>

    Redistribution and use in source and binary forms, with or without modification,
    are permitted provided that the following conditions are met:
    1. Redistributions of source code must retain the above copyright notice,
    this list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.
    3. Neither the name of the copyright holder nor the names of its contributors may
    be used to endorse or promote products derived from this software without specific
    prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
    IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
    INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
    BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
    DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
    LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
    OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
    ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "geomprober.h"

#include <QDebug>
#include <libgeom.h>

template<typename T>
QHash<QString, QString> getConfig(const T* p)
{
    QHash<QString, QString> config;
    gconfig* gc;

    LIST_FOREACH(gc, &p->lg_config, lg_config)
    {
        config[QString::fromLatin1(gc->lg_name)] = QString::fromLatin1(gc->lg_val);
    }

    return config;
}

void GeomProber::run()
{
    gmesh mesh;

    int error = geom_gettree(&mesh);
    if (error != 0)
    {
        qWarning() << "Cannot get GEOM tree";
        return;
    }

    QStringList labels;

    gclass* c = nullptr;
    LIST_FOREACH(c, &mesh.lg_class, lg_class)
    {
        if (strcmp(c->lg_name, "LABEL") == 0)
        {
            ggeom* g = nullptr;
            LIST_FOREACH(g, &c->lg_geom, lg_geom)
            {
                QString geomName = QString::fromLatin1(g->lg_name);
                if(geomName != m_dev)
                    continue;

                gprovider* p = nullptr;
                // for some reason there might be two geoms with the same name, but different labels
                LIST_FOREACH(p, &g->lg_provider, lg_provider)
                {
                    labels << QString::fromLatin1(p->lg_name);
                }
            }
        }
        else if (strcmp(c->lg_name, "DISK") == 0)
        {
            ggeom* g = nullptr;
            LIST_FOREACH(g, &c->lg_geom, lg_geom)
            {
                QString geomName = QString::fromLatin1(g->lg_name);
                if(geomName != m_dev && !m_dev.isEmpty())
                    continue;

                gprovider* p = g->lg_provider.lh_first;
                Q_ASSERT(p->lg_provider.le_next == nullptr);

                auto config = getConfig(p);
                // qDebug() << config;

                DiskInfo diskInfo;
                diskInfo.d->devName = geomName;
                diskInfo.d->descr = config["descr"];
                diskInfo.d->ident = config["ident"];
                diskInfo.d->mediaSize = p->lg_mediasize;
                diskInfo.d->sectorSize = p->lg_sectorsize;
                diskInfo.d->sectors = config["fwsectors"].toUInt();
                diskInfo.d->heads = config["fwheads"].toUInt();

                emit gotDisk(diskInfo);
                if(!m_dev.isEmpty())
                    break;
            }
        }
        else if (strcmp(c->lg_name, "PART") == 0)
        {
            ggeom* g = nullptr;
            LIST_FOREACH(g, &c->lg_geom, lg_geom)
            {
                QString geomName = QString::fromLatin1(g->lg_name);

                auto config = getConfig(g);

                PartTableInfo partInfo;
                partInfo.d->scheme = config["scheme"];

                config.clear();

                gprovider* p = nullptr;
                LIST_FOREACH(p, &g->lg_provider, lg_provider)
                {
                    Part part;

                    config = getConfig(p);

                    auto partName = QString::fromLatin1(p->lg_name);
                    part.d->length = config["length"].toULongLong();
                    part.d->offset = config["offset"].toUInt();
                    part.d->type = config["type"];
                    part.d->rawtype = config["rawtype"].toUShort();
                    part.d->index = config["index"].toUShort();

                    partInfo.d->partitions[partName] = part;

                    if(m_dev == partName)
                        emit gotPart(geomName, part);
                }

                if(m_dev.isEmpty() || m_dev == geomName)
                    emit gotPartTable(partInfo);
            }
        }
    }

    emit gotLabels(labels);
    emit finished();
    geom_deletetree(&mesh);
}

