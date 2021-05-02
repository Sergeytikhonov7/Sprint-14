#pragma once

#include "ranges.h"

#include <cstdlib>
#include <vector>

namespace graph {

    using VertexId = size_t;
    using EdgeId = size_t;

    template <typename Weight>
    struct Edge {
        VertexId from;
        VertexId to;
        Weight weight;
    };

    template <typename Weight>
    class DirectedWeightedGraph {
    private:
        using IncidenceList = std::vector<EdgeId>;
        using IncidentEdgesRange = ranges::Range<typename IncidenceList::const_iterator>;

    public:
        DirectedWeightedGraph() = default;

        explicit DirectedWeightedGraph(size_t vertex_count);

        EdgeId AddEdge(const Edge<Weight>& edge);

        size_t GetVertexCount() const;

        size_t GetEdgeCount() const;

        const Edge<Weight>& GetEdge(EdgeId edge_id) const;

        IncidentEdgesRange GetIncidentEdges(VertexId vertex) const;

        const std::vector<Edge<Weight>>& GetEdges() const {
            return edges_;
        }

        const std::vector<IncidenceList>& GetIncidenceLists() const {
            return incidence_lists_;
        }

        void SetEdge(Edge<Weight>& edge) {
            edges_.push_back(edge);
        }

        void SetIncidenceList(IncidenceList& edge) {
            incidence_lists_.push_back(std::move(edge));
        }

        void PrintDataOfGraph(std::ostream& out) const;

    private:
        std::vector<Edge<Weight>> edges_;
        std::vector<IncidenceList> incidence_lists_;
    };

    template <typename Weight>
    DirectedWeightedGraph<Weight>::DirectedWeightedGraph(size_t vertex_count)
            : incidence_lists_(vertex_count) {
    }

    template <typename Weight>
    EdgeId DirectedWeightedGraph<Weight>::AddEdge(const Edge<Weight>& edge) {
        edges_.push_back(edge);
        const EdgeId id = edges_.size() - 1;
        incidence_lists_.at(edge.from).push_back(id);
        return id;
    }

    template <typename Weight>
    size_t DirectedWeightedGraph<Weight>::GetVertexCount() const {
        return incidence_lists_.size();
    }

    template <typename Weight>
    size_t DirectedWeightedGraph<Weight>::GetEdgeCount() const {
        return edges_.size();
    }

    template <typename Weight>
    const Edge<Weight>& DirectedWeightedGraph<Weight>::GetEdge(EdgeId edge_id) const {
        return edges_.at(edge_id);
    }

    template <typename Weight>
    typename DirectedWeightedGraph<Weight>::IncidentEdgesRange
    DirectedWeightedGraph<Weight>::GetIncidentEdges(VertexId vertex) const {
        return ranges::AsRange(incidence_lists_.at(vertex));
    }

    template <typename Weight>
    void DirectedWeightedGraph<Weight>::PrintDataOfGraph(std::ostream& out) const {
        out << "Edges " << std::endl;
        for (const Edge<Weight>& edge: edges_) {
            out << edge.from << ' ' << edge.to << ' ' << edge.weight << std::endl;
        }
        out << std::endl;
        out << "IncidentLists " << std::endl;
        for (size_t i = 0; i < incidence_lists_.size(); ++i) {
            for (size_t j = 0; j < incidence_lists_[i].size(); ++j) {
                out << incidence_lists_[i][j] << std::endl;
            }
        }

    }
}  // namespace graph