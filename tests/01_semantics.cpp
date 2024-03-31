#include "TestContext.hpp"
#include "vuk/runtime/vk/AllocatorHelpers.hpp"
#include "vuk/vsl/Core.hpp"
#include <doctest/doctest.h>

using namespace vuk;

#include <string>

template<Access access = Access::eTransferWrite>
auto make_unary_void(std::string name, std::string& trace) {
	return make_pass(Name(name.c_str()), [=, &trace](CommandBuffer& cbuf, VUK_BA(access) dst) {
		trace += name;
		trace += " ";
	});
}

auto make_unary_computation(std::string name, std::string& trace) {
	return make_pass(Name(name.c_str()), [=, &trace](CommandBuffer& cbuf, VUK_BA(Access::eTransferWrite) dst) {
		trace += name;
		trace += " ";
		return dst;
	});
}

auto make_binary_computation(std::string name, std::string& trace) {
	return make_pass(Name(name.c_str()), [=, &trace](CommandBuffer& cbuf, VUK_BA(Access::eTransferRead) a, VUK_BA(Access::eTransferWrite) b) {
		trace += name;
		trace += " ";
		return a;
	});
}

TEST_CASE("conversion to SSA") {
	std::string trace = "";
	[[maybe_unused]] auto& oa = current_module;

	auto decl = declare_buf("_a", { .size = sizeof(uint32_t) * 4, .memory_usage = MemoryUsage::eGPUonly });
	make_unary_void("a", trace)(decl);
	make_unary_void("b", trace)(decl);
	make_unary_void<Access::eTransferRead>("c", trace)(decl);
	decl.submit(*test_context.allocator, test_context.compiler);

	trace = trace.substr(0, trace.size() - 1);
	CHECK(trace == "a b");
}

TEST_CASE("minimal graph is submitted") {
	[[maybe_unused]] auto& oa = current_module;

	for (int i = 0; i < 32; i++) {
		fmt::println("{}\n", current_module->op_arena.size());
		std::string trace = "";

		auto a = make_unary_computation("a", trace)(declare_buf("_a", { .size = sizeof(uint32_t) * 4, .memory_usage = MemoryUsage::eGPUonly }));
		auto b = make_unary_computation("b", trace)(declare_buf("_b", { .size = sizeof(uint32_t) * 4, .memory_usage = MemoryUsage::eGPUonly }));

		auto d = make_binary_computation("d", trace)(a, b); // d->a, d->b
		auto e = make_unary_computation("e", trace)(a);     // e->a
		e.submit(*test_context.allocator, test_context.compiler);

		trace = trace.substr(0, trace.size() - 1);
		CHECK(trace == "a e");
	}
}

TEST_CASE("graph is cleaned up after submit") {
	std::string trace = "";
	[[maybe_unused]] auto& oa = current_module->op_arena;
	CHECK(current_module->op_arena.size() == 0);

	auto a = make_unary_computation("a", trace)(declare_buf("_a", { .size = sizeof(uint32_t) * 4, .memory_usage = MemoryUsage::eGPUonly }));
	// auto b = make_unary_computation("b", trace)(declare_buf("_b", { .size = sizeof(uint32_t) * 4, .memory_usage = MemoryUsage::eGPUonly }));

	// auto d = make_binary_computation("d", trace)(a, b); // d->a, d->b
	auto e = make_unary_computation("e", trace)(a); // e->a
	e.submit(*test_context.allocator, test_context.compiler);

	current_module->collect_garbage();
	for (auto& op : current_module->op_arena) {
		fmt::println("{}", op.kind_to_sv());
	}
#ifndef VUK_GARBAGE_SAN
	CHECK(current_module->op_arena.size() == 2);
#endif
}

TEST_CASE("computation is never duplicated") {
	std::string trace = "";

	auto a = make_unary_computation("a", trace)(declare_buf("_a", { .size = sizeof(uint32_t) * 4, .memory_usage = MemoryUsage::eGPUonly }));
	auto b = make_unary_computation("b", trace)(declare_buf("_b", { .size = sizeof(uint32_t) * 4, .memory_usage = MemoryUsage::eGPUonly }));

	auto d = make_binary_computation("d", trace)(a, b); // d->a, d->b
	auto e = make_unary_computation("e", trace)(a);     // e->a

	e.submit(*test_context.allocator, test_context.compiler);
	d.submit(*test_context.allocator, test_context.compiler);
	trace = trace.substr(0, trace.size() - 1);
	CHECK(trace == "a e b d");
}

TEST_CASE("computation is never duplicated 2") {
	std::string trace = "";

	auto a = make_unary_computation("a", trace)(declare_buf("_a", { .size = sizeof(uint32_t) * 4, .memory_usage = MemoryUsage::eGPUonly }));
	auto b = make_unary_computation("b", trace)(declare_buf("_b", { .size = sizeof(uint32_t) * 4, .memory_usage = MemoryUsage::eGPUonly }));

	auto d = make_binary_computation("d", trace)(a, b); // d->a, d->b
	d.submit(*test_context.allocator, test_context.compiler);
	d.submit(*test_context.allocator, test_context.compiler);
	trace = trace.substr(0, trace.size() - 1);
	bool good = (trace == "a b d") || (trace == "b a d");
	CHECK(good);
}

TEST_CASE("computation is never duplicated 3") {
	std::string trace = "";

	auto a = make_unary_computation("a", trace)(declare_buf("_a", { .size = sizeof(uint32_t) * 4, .memory_usage = MemoryUsage::eGPUonly }));
	auto b = make_unary_computation("b", trace)(declare_buf("_b", { .size = sizeof(uint32_t) * 4, .memory_usage = MemoryUsage::eGPUonly }));

	auto [ap, bp] = make_pass("d", [=, &trace](CommandBuffer& cbuf, VUK_BA(Access::eTransferWrite) a, VUK_BA(Access::eTransferWrite) b) {
		trace += "d";
		trace += " ";
		return std::make_tuple(a, b);
	})(a, b);

	ap.submit(*test_context.allocator, test_context.compiler);
	bp.submit(*test_context.allocator, test_context.compiler);
	trace = trace.substr(0, trace.size() - 1);
	bool good = (trace == "a b d") || (trace == "b a d");
	CHECK(good);
}

TEST_CASE("not moving Values will emit splices") {
	std::string trace = "";

	auto a = make_unary_computation("a", trace)(declare_buf("_a", { .size = sizeof(uint32_t) * 4, .memory_usage = MemoryUsage::eGPUonly }));
	auto b = make_unary_computation("b", trace)(declare_buf("_b", { .size = sizeof(uint32_t) * 4, .memory_usage = MemoryUsage::eGPUonly }));

	auto d = make_binary_computation("d", trace)(a, b); // d->a, d->b
	d.submit(*test_context.allocator, test_context.compiler);
	trace = trace.substr(0, trace.size() - 1);
	bool good = (trace == "a b d") || (trace == "b a d");
	CHECK(good);
}

TEST_CASE("moving Values allows for more efficient building (but no semantic change)") {
	std::string trace = "";

	auto a = make_unary_computation("a", trace)(declare_buf("_a", { .size = sizeof(uint32_t) * 4, .memory_usage = MemoryUsage::eGPUonly }));
	auto b = make_unary_computation("b", trace)(declare_buf("_b", { .size = sizeof(uint32_t) * 4, .memory_usage = MemoryUsage::eGPUonly }));

	auto d = make_binary_computation("d", trace)(std::move(a), std::move(b)); // d->a, d->b
	d.submit(*test_context.allocator, test_context.compiler);
	trace = trace.substr(0, trace.size() - 1);
	bool good = (trace == "a b d") || (trace == "b a d");
	CHECK(good);
}

TEST_CASE("moving Values doesn't help if it was leaked before") {
	std::string trace = "";

	auto a = make_unary_computation("a", trace)(declare_buf("_a", { .size = sizeof(uint32_t) * 4, .memory_usage = MemoryUsage::eGPUonly }));
	auto b = make_unary_computation("b", trace)(declare_buf("_b", { .size = sizeof(uint32_t) * 4, .memory_usage = MemoryUsage::eGPUonly }));

	auto d = make_binary_computation("d", trace)(a, b);        // d->a, d->b
	auto e = make_unary_computation("e", trace)(std::move(a)); // e->a <--- a cannot be consumed here! since previously we made d depend on a
	e.submit(*test_context.allocator, test_context.compiler);
	d.submit(*test_context.allocator, test_context.compiler);
	trace = trace.substr(0, trace.size() - 1);
	CHECK(trace == "a e b d");
}

TEST_CASE("scheduling single-queue") {
	std::string execution;

	auto buf0 = allocate_buffer(*test_context.allocator, { .mem_usage = MemoryUsage::eGPUonly, .size = sizeof(uint32_t) * 4 });

	auto write = make_pass("write", [&](CommandBuffer& cbuf, VUK_BA(Access::eTransferWrite) dst) {
		execution += "w";
		return dst;
	});
	auto write2 = make_pass("write2", [&](CommandBuffer& cbuf, VUK_BA(Access::eTransferWrite) dst) {
		execution += "w";
		return dst;
	});
	auto read = make_pass("read", [&](CommandBuffer& cbuf, VUK_BA(Access::eTransferRead) dst) {
		execution += "r";
		return dst;
	});

	{
		auto b0 = discard_buf("src0", **buf0);
		write(write(b0)).wait(*test_context.allocator, test_context.compiler);
		CHECK(execution == "ww");
		execution = "";
	}
	{
		auto b0 = discard_buf("src0", **buf0);
		read(write(b0)).wait(*test_context.allocator, test_context.compiler);
		CHECK(execution == "wr");
		execution = "";
	}
	{
		auto b0 = discard_buf("src0", **buf0);
		write2(read(write(b0))).wait(*test_context.allocator, test_context.compiler);
		CHECK(execution == "wrw");
		execution = "";
	}
	{
		auto b0 = discard_buf("src0", **buf0);
		write(read(read(write(b0)))).wait(*test_context.allocator, test_context.compiler);
		CHECK(execution == "wrrw");
	}
}

TEST_CASE("write-read-write") {
	std::string execution;

	for (int i = 0; i < 32; i++) {
		auto buf0 = allocate_buffer(*test_context.allocator, { .mem_usage = MemoryUsage::eGPUonly, .size = sizeof(uint32_t) * 4 });
		auto buf1 = allocate_buffer(*test_context.allocator, { .mem_usage = MemoryUsage::eGPUonly, .size = sizeof(uint32_t) * 4 });
		auto buf2 = allocate_buffer(*test_context.allocator, { .mem_usage = MemoryUsage::eGPUonly, .size = sizeof(uint32_t) * 4 });

		auto write = make_pass("write", [&](CommandBuffer& cbuf, VUK_BA(Access::eTransferWrite) dst) {
			execution += "w";
			return dst;
		});
		auto write2 = make_pass("write", [&](CommandBuffer& cbuf, VUK_BA(Access::eTransferWrite) dst, VUK_BA(Access::eTransferRead) dst2) {
			execution += "w";
			return dst;
		});
		auto read = make_pass("read", [&](CommandBuffer& cbuf, VUK_BA(Access::eTransferWrite) dst, VUK_BA(Access::eTransferRead) r) {
			execution += "r";
			return dst;
		});

		{
			auto b0 = discard_buf("src0", **buf0);
			auto b1 = discard_buf("src1", **buf1);
			auto b2 = discard_buf("src2", **buf2);
			b0 = write(b0);
			b1 = write(b1);
			b2 = write(b2);
			auto b0p = read(b0, b1);
			auto b2p = read(b2, b1);
			write2(b0p, b2p).wait(*test_context.allocator, test_context.compiler);
			CHECK(execution == "wwrwrw");
			execution = "";
		}
	}
}

TEST_CASE("scheduling with submitted") {
	std::string execution;

	auto buf0 = allocate_buffer(*test_context.allocator, { .mem_usage = MemoryUsage::eGPUonly, .size = sizeof(uint32_t) * 4 });
	auto buf1 = allocate_buffer(*test_context.allocator, { .mem_usage = MemoryUsage::eGPUonly, .size = sizeof(uint32_t) * 4 });

	auto write = make_pass("write", [&](CommandBuffer& cbuf, VUK_BA(Access::eTransferWrite) dst) {
		execution += "w";
		return dst;
	});
	auto read = make_pass("read", [&](CommandBuffer& cbuf, VUK_BA(Access::eTransferRead) dst) {
		execution += "r";
		return dst;
	});
	auto read2 = make_pass("read", [&](CommandBuffer& cbuf, VUK_BA(Access::eTransferRead) dst, VUK_BA(Access::eTransferRead) dst2) {
		execution += "r";
		return dst;
	});

	{
		auto written = write(discard_buf("src0", **buf0));
		written.wait(*test_context.allocator, test_context.compiler);
		{
			auto buf2 = discard_buf("src1", **buf1);
			auto res = read2(write(buf2), written);
			res.wait(*test_context.allocator, test_context.compiler);
		}
		{
			auto res2 = read(written);
			res2.wait(*test_context.allocator, test_context.compiler);
		}
		CHECK(execution == "wwrr");
		execution = "";
	}

	{
		auto written = write(discard_buf("src0", **buf0));
		written.wait(*test_context.allocator, test_context.compiler);
		read(written).wait(*test_context.allocator, test_context.compiler);
		CHECK(execution == "wr");
		execution = "";
	}
	{
		auto written = write(discard_buf("src0", **buf0));
		written.wait(*test_context.allocator, test_context.compiler);
		read(std::move(written)).wait(*test_context.allocator, test_context.compiler);
		CHECK(execution == "wr");
		execution = "";
	}
	{
		auto written = write(discard_buf("src0", **buf0));
		written.wait(*test_context.allocator, test_context.compiler);
		auto res = write(std::move(written));
		res.wait(*test_context.allocator, test_context.compiler);
		CHECK(execution == "ww");
		execution = "";
	}
}

TEST_CASE("multi-queue buffers") {
	std::string execution;

	auto buf0 = allocate_buffer(*test_context.allocator, { .mem_usage = MemoryUsage::eGPUonly, .size = sizeof(uint32_t) * 4 });

	auto write = make_pass(
	    "write_A",
	    [&](CommandBuffer& cbuf, VUK_BA(Access::eTransferWrite) dst) {
		    cbuf.fill_buffer(dst, 0xf);
		    execution += "w";
		    CHECK((cbuf.get_scheduled_domain() & DomainFlagBits::eTransferQueue).m_mask != 0);
		    return dst;
	    },
	    DomainFlagBits::eTransferQueue);
	auto write2 = make_pass(
	    "write_A",
	    [&](CommandBuffer& cbuf, VUK_BA(Access::eTransferWrite) dst) {
		    cbuf.fill_buffer(dst, 0xf);
		    execution += "w";
		    CHECK((cbuf.get_scheduled_domain() & DomainFlagBits::eGraphicsQueue).m_mask != 0);
		    return dst;
	    },
	    DomainFlagBits::eTransferQueue);
	auto read = make_pass(
	    "read_B",
	    [&](CommandBuffer& cbuf, VUK_BA(Access::eTransferRead) dst) {
		    auto dummy = allocate_buffer(*test_context.allocator, { .mem_usage = MemoryUsage::eGPUonly, .size = sizeof(uint32_t) * 4 });
		    cbuf.copy_buffer(**dummy, dst);
		    execution += "r";
		    CHECK((cbuf.get_scheduled_domain() & DomainFlagBits::eGraphicsQueue).m_mask != 0);
		    return dst;
	    },
	    DomainFlagBits::eGraphicsQueue);

	{
		CHECK(current_module->op_arena.size() == 0);
		auto written = write(discard_buf("src0", **buf0));
		written.wait(*test_context.allocator, test_context.compiler);
		read(written).wait(*test_context.allocator, test_context.compiler);
		CHECK(execution == "wr");
		execution = "";
	}
	{
#ifndef VUK_GARBAGE_SAN
		CHECK(current_module->op_arena.size() == 3);
#endif
		auto written = write(discard_buf("src0", **buf0));
		written.wait(*test_context.allocator, test_context.compiler);
		read(std::move(written)).wait(*test_context.allocator, test_context.compiler);
		CHECK(execution == "wr");
		execution = "";
	}
	{
#ifndef VUK_GARBAGE_SAN
		CHECK(current_module->op_arena.size() == 3);
#endif
		auto written = write(discard_buf("src0", **buf0));
		written.wait(*test_context.allocator, test_context.compiler);
		write2(read(std::move(written))).wait(*test_context.allocator, test_context.compiler);
		CHECK(execution == "wrw");
		execution = "";
	}
	{
#ifndef VUK_GARBAGE_SAN
		// CHECK(current_module->op_arena.size() == 3);
#endif
		auto written = write(discard_buf("src0", **buf0));
		read(written).wait(*test_context.allocator, test_context.compiler);
		CHECK(execution == "wr");
		execution = "";
	}
	{
#ifndef VUK_GARBAGE_SAN
		// CHECK(current_module->op_arena.size() == 3);
#endif
		auto written = write(discard_buf("src0", **buf0));
		read(std::move(written)).wait(*test_context.allocator, test_context.compiler);
		CHECK(execution == "wr");
		execution = "";
	}
	{
#ifndef VUK_GARBAGE_SAN
		// CHECK(current_module->op_arena.size() == 1);
#endif
		auto written = write(discard_buf("src0", **buf0));
		write2(read(std::move(written))).wait(*test_context.allocator, test_context.compiler);
		CHECK(execution == "wrw");
		execution = "";
	}
}

TEST_CASE("queue inference") {
	std::string execution;

	auto buf0 = allocate_buffer(*test_context.allocator, { .mem_usage = MemoryUsage::eGPUonly, .size = sizeof(uint32_t) * 4 });

	auto transfer = make_pass(
	    "transfer",
	    [&](CommandBuffer& cbuf, VUK_BA(Access::eTransferWrite) dst) {
		    cbuf.fill_buffer(dst, 0xf);
		    execution += "t";
		    CHECK((cbuf.get_scheduled_domain() & DomainFlagBits::eTransferQueue).m_mask != 0);
		    return dst;
	    },
	    DomainFlagBits::eTransferQueue);

	auto neutral = make_pass("neutral", [&](CommandBuffer& cbuf, VUK_BA(Access::eTransferWrite) dst) {
		cbuf.fill_buffer(dst, 0xf);
		execution += "n";
		CHECK((cbuf.get_scheduled_domain() & DomainFlagBits::eTransferQueue).m_mask != 0);
		return dst;
	});

	auto gfx = make_pass(
	    "gfx",
	    [&](CommandBuffer& cbuf, VUK_BA(Access::eTransferWrite) dst) {
		    auto dummy = allocate_buffer(*test_context.allocator, { .mem_usage = MemoryUsage::eGPUonly, .size = sizeof(uint32_t) * 4 });
		    cbuf.copy_buffer(**dummy, dst);
		    execution += "g";
		    CHECK((cbuf.get_scheduled_domain() & DomainFlagBits::eGraphicsQueue).m_mask != 0);
		    return dst;
	    },
	    DomainFlagBits::eGraphicsQueue);

	{
		CHECK(current_module->op_arena.size() == 0);
		auto written = gfx(neutral(transfer(discard_buf("src0", **buf0))));
		written.wait(*test_context.allocator, test_context.compiler);
		CHECK(execution == "tng");
	}
}

TEST_CASE("multi return pass") {
	auto buf0 = allocate_buffer(*test_context.allocator, { .mem_usage = MemoryUsage::eGPUonly, .size = sizeof(uint32_t) * 4 });
	auto buf1 = allocate_buffer(*test_context.allocator, { .mem_usage = MemoryUsage::eGPUonly, .size = sizeof(uint32_t) * 4 });
	auto buf2 = allocate_buffer(*test_context.allocator, { .mem_usage = MemoryUsage::eGPUonly, .size = sizeof(uint32_t) * 4 });

	auto fills = make_pass(
	    "fills", [](CommandBuffer& cbuf, VUK_BA(Access::eTransferWrite) dst0, VUK_BA(Access::eTransferWrite) dst1, VUK_BA(Access::eTransferWrite) dst2) {
		    cbuf.fill_buffer(dst0, 0xfc);
		    cbuf.fill_buffer(dst1, 0xfd);
		    cbuf.fill_buffer(dst2, 0xfe);
		    return std::tuple{ dst0, dst1, dst2 };
	    });

	auto [buf0p, buf1p, buf2p] = fills(discard_buf("src0", **buf0), discard_buf("src1", **buf1), discard_buf("src2", **buf2));
	{
		auto data = { 0xfcu, 0xfcu, 0xfcu, 0xfcu };
		auto res = download_buffer(buf0p).get(*test_context.allocator, test_context.compiler);
		CHECK(std::span((uint32_t*)res->mapped_ptr, 4) == std::span(data));
	}
	{
		auto data = { 0xfdu, 0xfdu, 0xfdu, 0xfdu };
		auto res = download_buffer(buf1p).get(*test_context.allocator, test_context.compiler);
		CHECK(std::span((uint32_t*)res->mapped_ptr, 4) == std::span(data));
	}
	{
		auto data = { 0xfeu, 0xfeu, 0xfeu, 0xfeu };
		auto res = download_buffer(buf2p).get(*test_context.allocator, test_context.compiler);
		CHECK(std::span((uint32_t*)res->mapped_ptr, 4) == std::span(data));
	}
}

TEST_CASE("multi fn calls") {
	auto buf0 = allocate_buffer(*test_context.allocator, { .mem_usage = MemoryUsage::eGPUonly, .size = sizeof(uint32_t) * 4 });

	std::unique_ptr<int> a = std::make_unique<int>(5);
	auto p = make_pass("fills", [ab = std::move(a)](CommandBuffer& cbuf, VUK_BA(Access::eTransferWrite) dst0) {
		CHECK(ab);
		return dst0;
	});

	p(p(discard_buf("src0", **buf0))).get(*test_context.allocator, test_context.compiler);
}